// Copyright eeGeo Ltd (2012-2017), All Rights Reserved

#include <array>
#include <functional>

#include "ApplicationConfig.h"
#include "AutomatedScreenshotController.h"
#include "EegeoWorld.h"
#include "GlobeCameraController.h"
#include "ICallback.h"
#include "ICameraTransitionController.h"
#include "IFlattenButtonModel.h"
#include "InteriorsCameraController.h"
#include "InteriorsExplorerModel.h"
#include "InteriorSelectionModel.h"
#include "IPlaceJumpController.h"
#include "IScreenshotService.h"
#include "ISearchQueryPerformer.h"
#include "ISearchResultPoiViewModel.h"
#include "IWeatherController.h"
#include "OpenSearchMenuMessage.h"
#include "PlaneSimulation.h"
#include "StreamingController.h"
#include "TimeHelpers.h"

namespace ExampleApp
{
    namespace Automation
    {
        namespace
        {
            const int UpdateCyclesToWaitForSplashScreenToDisappear = 80;
            const int UpdateCyclesPerScreenshot = 25;

            enum {
                ExecStateStreamingInitialSceneAndManifest,
                ExecStateWaitingForSceneToStream,
                ExecStateWaitForSceneToRender,
                ExecStatePrepareNextScene,
                ExecStateDone
            };

            const AutomatedScreenshotController::WaitPredicate NoWait([]() { return true; });
            const AutomatedScreenshotController::WaitPredicate WaitMs(const long long msToWait)
            {
                const long long currentMillis = Eegeo::Helpers::Time::MillisecondsSinceEpoch();
                return [=]() {
                    return Eegeo::Helpers::Time::MillisecondsSinceEpoch() - currentMillis > msToWait;
                };
            }

            template <typename F>
            const AutomatedScreenshotController::WaitPredicate Act(F f)
            {
                return [=]() {
                    f();
                    return true;
                };
            }

            const AutomatedScreenshotController::WaitPredicate Then(AutomatedScreenshotController::WaitPredicate p1, AutomatedScreenshotController::WaitPredicate p2)
            {
                bool p1Done = false;
                return [=]() mutable {
                    if (p1Done)
                    {
                        p1Done = p1();
                        return false;
                    }
                    else
                    {
                        return p2();
                    }
                };
            }
        }

        AutomatedScreenshotController::AutomatedScreenshotController(const ExampleApp::ApplicationConfig::ApplicationConfiguration& applicationConfiguration,
                                                                     ExampleApp::CameraTransitions::SdkModel::ICameraTransitionController& cameraTransitionController,
                                                                     const Eegeo::Camera::GlobeCamera::GlobeCameraController& globeCameraController,
                                                                     Eegeo::Traffic::PlaneSimulation &planeSimulation,
                                                                     ExampleApp::PlaceJumps::SdkModel::IPlaceJumpController& placeJumpController,
                                                                     ExampleApp::WeatherMenu::SdkModel::IWeatherController& weatherController,
                                                                     ExampleApp::Search::SdkModel::ISearchQueryPerformer& searchQueryPerformer,
                                                                     ExampleApp::FlattenButton::SdkModel::IFlattenButtonModel& flattenButtonModel,
                                                                     ExampleApp::SearchResultPoi::View::ISearchResultPoiViewModel& searchResultPoiViewModel,
                                                                     Eegeo::Resources::Interiors::InteriorSelectionModel& interiorSelectionModel,
                                                                     Eegeo::Resources::Interiors::InteriorsCameraController& interiorCameraController,
                                                                     ExampleApp::InteriorsExplorer::SdkModel::InteriorsExplorerModel& interiorsExplorerModel,
                                                                     Eegeo::Streaming::StreamingController& streamingController,
                                                                     IScreenshotService& screenshotService,
                                                                     Eegeo::EegeoWorld& eegeoWorld,
                                                                     ExampleAppMessaging::TMessageBus& messageBus)
        : m_planeSimulation(planeSimulation)
        , m_placeJumpController(placeJumpController)
        , m_weatherController(weatherController)
        , m_searchQueryPerformer(searchQueryPerformer)
        , m_flattenButtonModel(flattenButtonModel)
        , m_screenshotService(screenshotService)
        , m_eegeoWorld(eegeoWorld)
        , m_updateCyclesToWait(UpdateCyclesPerScreenshot)
        , m_execState(ExecStateStreamingInitialSceneAndManifest)
        , m_sceneIndex(0)
        , m_messageBus(messageBus)
        {
            m_planeSimulation.SetEnabled(false);
        }

        bool AutomatedScreenshotController::NextScene()
        {
            if (m_sceneIndex < NumScenes())
            {
                m_waitPredicate = SetupState(m_sceneIndex);
                m_sceneIndex++;
                return true;
            }
            else
            {
                return false;
            }
        }

        const std::array<AutomatedScreenshotController::SceneSetupFunction, 3> AutomatedScreenshotController::States() const
        {
            return {{
                [this]() {
                    const PlaceJumps::View::PlaceJumpModel GoldenGateBridge(
                            "SanFranGoldenGate",
                            Eegeo::Space::LatLong::FromDegrees(37.81588, -122.47787),
                            336.0,
                            1800,
                            "");

                    m_flattenButtonModel.Unflatten();
                    m_placeJumpController.JumpTo(GoldenGateBridge);
                    m_weatherController.SetTime("Day");
                    m_weatherController.SetTheme("Autumn");

                    return WaitMs(10000);
                },

                [this]() {
                    const PlaceJumps::View::PlaceJumpModel NewYorkFinancialDistrict(
                            "NewYorkFinancialDistrict",
                            Eegeo::Space::LatLong::FromDegrees(40.708798, -74.010326),
                            0.000000f,
                            2597.526123f,
                            "");

                    m_placeJumpController.JumpTo(NewYorkFinancialDistrict);

                    m_weatherController.SetTime("Dawn");
                    m_weatherController.SetTheme("Summer");
                    m_flattenButtonModel.Unflatten();

                    return NoWait;
                },

                [this]() {
                    const long long MsToWaitForSearchResults = 3000;
                    const long long MsToWaitForSidebarAnimation = 8000;
                    const Eegeo::Space::LatLong location(Eegeo::Space::LatLong::FromDegrees(55.948685, -3.201244));
                    const PlaceJumps::View::PlaceJumpModel EdinburghCastle(
                            "EdinburghCastle",
                            location,
                            0.000000f,
                            800.0f,
                            "");

                    m_placeJumpController.JumpTo(EdinburghCastle);

                    m_weatherController.SetTime("Day");
                    m_weatherController.SetTheme("Summer");
                    m_flattenButtonModel.Unflatten();

                    m_searchQueryPerformer.PerformSearchQuery("coffee", false, false, Eegeo::Space::LatLongAltitude(location.GetLatitude(), location.GetLongitude(), 0.0f));

                    return Then(WaitMs(MsToWaitForSearchResults),
                           Then(Act([this]() { m_messageBus.Publish(SearchMenu::OpenSearchMenuMessage(true)); }),
                                WaitMs(MsToWaitForSidebarAnimation)));
                }
            }};
        }

        const unsigned long AutomatedScreenshotController::NumScenes() const
        {
            return States().size();
        }

        std::function<bool()> AutomatedScreenshotController::SetupState(const unsigned long state)
        {
            return States()[state]();
        }

        bool AutomatedScreenshotController::Done() const
        {
            return m_sceneIndex == NumScenes();
        }

        void AutomatedScreenshotController::Update(const float dt)
        {
            switch (m_execState)
            {
            case ExecStateStreamingInitialSceneAndManifest:
                if (!m_eegeoWorld.IsStreaming(false))
                {
                    NextScene();
                    m_execState = ExecStateWaitingForSceneToStream;
                    m_updateCyclesToWait = UpdateCyclesToWaitForSplashScreenToDisappear;
                }
                return;

            case ExecStateWaitingForSceneToStream:
                if (!m_eegeoWorld.IsStreaming(false))
                {
                    m_execState = ExecStateWaitForSceneToRender;
                }
                return;

            case ExecStateWaitForSceneToRender:
                if (m_updateCyclesToWait <= 0 && m_waitPredicate())
                {
                    m_screenshotService.screenshot("screenshot");
                    m_execState = ExecStatePrepareNextScene;
                }

                m_updateCyclesToWait--;
                return;

            case ExecStatePrepareNextScene:
                if (Done())
                {
                    m_completedScreenshotsCallbacks.ExecuteCallbacks();
                    m_execState = ExecStateDone;
                }
                else
                {
                    NextScene();
                    m_updateCyclesToWait = UpdateCyclesPerScreenshot;
                    m_execState = ExecStateWaitingForSceneToStream;
                }
                return;

            case ExecStateDone:
            default:
                return;
            }
        }

        void AutomatedScreenshotController::InsertCompletedScreenshotsCallback(Eegeo::Helpers::ICallback0& callback)
        {
            m_completedScreenshotsCallbacks.AddCallback(callback);
        }

        void AutomatedScreenshotController::RemoveCompletedScreenshotsCallback(Eegeo::Helpers::ICallback0& callback)
        {
            m_completedScreenshotsCallbacks.RemoveCallback(callback);
        }
    }
}
