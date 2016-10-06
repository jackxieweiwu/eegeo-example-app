// Copyright eeGeo Ltd (2012-2015), All Rights Reserved

#include "AppHost.h"
#include "AndroidSharedGlContext.h"
#include "LatLongAltitude.h"
#include "EegeoWorld.h"
#include "RenderContext.h"
#include "GlobalLighting.h"
#include "GlobalFogging.h"
#include "AppInterface.h"
#include "JpegLoader.h"
#include "EffectHandler.h"
#include "SearchServiceCredentials.h"
#include "AndroidThreadHelper.h"
#include "GlobeCameraController.h"
#include "RenderCamera.h"
#include "CameraHelpers.h"
#include "LoadingScreen.h"
#include "PlatformConfig.h"
#include "AndroidPlatformConfigBuilder.h"
#include "AndroidUrlEncoder.h"
#include "AndroidFileIO.h"
#include "AndroidLocationService.h"
#include "EegeoWorld.h"
#include "EnvironmentFlatteningService.h"
#include "TtyHandler.h"
#include "MenuViewModule.h"
#include "SettingsMenuModule.h"
#include "ModalityModule.h"
#include "ModalBackgroundViewModule.h"
#include "ModalBackgroundNativeViewModule.h"
#include "MenuModel.h"
#include "MenuViewModel.h"
#include "SearchMenuModule.h"
#include "MenuOptionsModel.h"
#include "SearchModule.h"
#include "SearchResultOnMapModule.h"
#include "WorldPinsModule.h"
#include "RegularTexturePageLayout.h"
#include "PinsModule.h"
#include "SearchResultRepository.h"
#include "LatLongAltitude.h"
#include "SearchResultPoiModule.h"
#include "AndroidPlatformAbstractionModule.h"
#include "FlattenButtonModule.h"
#include "FlattenButtonViewModule.h"
#include "SearchResultPoiViewModule.h"
#include "WorldPinOnMapViewModule.h"
#include "PlaceJumpsModule.h"
#include "IPlaceJumpController.h"
#include "SettingsMenuViewModule.h"
#include "SearchMenuViewModule.h"
#include "CompassViewModule.h"
#include "CompassModule.h"
#include "AboutPageViewModule.h"
#include "AndroidInitialExperienceModule.h"
#include "ViewControllerUpdaterModule.h"
#include "ViewControllerUpdaterModel.h"
#include "TagSearchModule.h"
#include "ScreenProperties.h"
#include "MyPinCreationViewModule.h"
#include "MyPinCreationDetailsViewModule.h"
#include "MyPinDetailsViewModule.h"
#include "InitialExperienceIntroViewModule.h"
#include "Logger.h"
#include "AndroidAppThreadAssertionMacros.h"
#include "SearchResultRepositoryObserver.h"
#include "ApiKey.h"
#include "OptionsViewModule.h"
#include "OptionsView.h"
#include "WatermarkViewModule.h"
#include "WatermarkView.h"
#include "NetworkCapabilities.h"
#include "ApplicationConfigurationModule.h"
#include "IApplicationConfigurationService.h"
#include "SearchVendorNames.h"
#include "UserInteractionEnabledChangedMessage.h"
#include "AndroidApplicationConfigurationVersionProvider.h"
#include "InteriorsExplorerModule.h"
#include "InteriorsExplorerViewModule.h"
#include "SearchResultSectionModule.h"
#include "ConnectivityChangedViewMessage.h"
#include "WebConnectivityValidator.h"
#include "AndroidMenuReactionModel.h"
#include "ApplicationConfigurationModule.h"
#include "ILocationService.h"
#include "AppWiring.h"
#include "MobileExampleApp.h"
#include "AndroidAppModule.h"
#include "IPersistentSettingsModel.h"

using namespace Eegeo::Android;
using namespace Eegeo::Android::Input;

AppHost::AppHost(
    AndroidNativeState& nativeState,
    Eegeo::Rendering::ScreenProperties screenProperties,
    EGLDisplay display,
    EGLSurface shareSurface,
    EGLContext resourceBuildShareContext
)
    :m_isPaused(false)
    ,m_nativeState(nativeState)
    ,m_androidInputBoxFactory(&nativeState)
    ,m_androidKeyboardInputFactory(&nativeState, m_inputHandler)
    ,m_androidAlertBoxFactory(&nativeState)
    ,m_androidNativeUIFactories(m_androidAlertBoxFactory, m_androidInputBoxFactory, m_androidKeyboardInputFactory)
    ,m_registeredUIModules(false)
    ,m_resolvedUIModules(false)
    ,m_requestedApplicationInitialiseViewState(false)
    ,m_uiCreatedMessageReceivedOnNativeThread(false)
	,m_userInteractionEnabledChangedHandler(this, &AppHost::HandleUserInteractionEnabledChanged)
    ,m_app(nullptr)
    ,m_wiring(nullptr)
{
    ASSERT_NATIVE_THREAD

    Eegeo_ASSERT(resourceBuildShareContext != EGL_NO_CONTEXT);

    Eegeo::TtyHandler::TtyEnabled = true;
    Eegeo::AssertHandler::BreakOnAssert = true;


    m_wiring = std::make_shared<ExampleApp::AppWiring>();
    m_wiring->RegisterModuleInstance(std::make_shared<ExampleApp::Android::AndroidAppModule>(nativeState, screenProperties, display, shareSurface, resourceBuildShareContext));
    m_wiring->RegisterModule<ExampleApp::Watermark::View::WatermarkViewModule>();
    m_wiring->RegisterModule<ExampleApp::WorldPins::View::WorldPinOnMapViewModule>();
    m_wiring->RegisterModule<ExampleApp::FlattenButton::View::FlattenButtonViewModule>();
    m_wiring->RegisterModule<ExampleApp::MyPinCreation::View::MyPinCreationViewModule>();
    m_wiring->RegisterModule<ExampleApp::Compass::View::CompassViewModule>();
    m_wiring->RegisterModule<ExampleApp::ModalBackground::View::ModalBackgroundViewModule>();
    m_wiring->RegisterModule<ExampleApp::SearchMenu::View::SearchMenuViewModule>();
    m_wiring->RegisterModule<ExampleApp::SettingsMenu::View::SettingsMenuViewModule>();
    m_wiring->RegisterModule<ExampleApp::SearchResultPoi::View::SearchResultPoiViewModule>();
    m_wiring->RegisterModule<ExampleApp::AboutPage::View::AboutPageViewModule>();
    m_wiring->RegisterModule<ExampleApp::Options::View::OptionsViewModule>();
    m_wiring->RegisterModule<ExampleApp::MyPinCreationDetails::View::MyPinCreationDetailsViewModule>();
    m_wiring->RegisterModule<ExampleApp::MyPinDetails::View::MyPinDetailsViewModule>();
    m_wiring->RegisterModule<ExampleApp::InitialExperience::View::InitialExperienceIntroViewModule>();
    m_wiring->RegisterModule<ExampleApp::InteriorsExplorer::View::InteriorsExplorerViewModule>();
    m_wiring->RegisterModule<ExampleApp::ViewControllerUpdater::View::ViewControllerUpdaterModule>();
    m_wiring->RegisterDefaultModules();
    m_wiring->ApplyModuleRegistrations();
    m_wiring->BuildContainer();
	m_wiring->ResolveLeaf<ExampleApp::ApplicationConfig::ApplicationConfiguration>();
	m_wiring->ResolveLeaf<Eegeo::Modules::IPlatformAbstractionModule>();
	m_wiring->ResolveLeaf<Eegeo::Web::IConnectivityService>();
	m_wiring->ResolveLeaf<ExampleApp::PersistentSettings::IPersistentSettingsModel>();
	m_wiring->ResolveLeaf<ExampleApp::Net::SdkModel::INetworkCapabilities>();
	m_wiring->ResolveLeaf<Eegeo::EegeoWorld>();
}

AppHost::~AppHost()
{
    ASSERT_NATIVE_THREAD

    m_inputHandler.RemoveDelegateInputHandler(m_pAppInputDelegate);

    Eegeo_DELETE m_pAppInputDelegate;
    m_pAppInputDelegate = NULL;
}

void AppHost::OnResume()
{
    ASSERT_NATIVE_THREAD

    m_app->OnResume();
    m_isPaused = false;
}

void AppHost::OnPause()
{
    ASSERT_NATIVE_THREAD

    m_isPaused = true;
    m_app->OnPause();
    m_wiring->Resolve<Eegeo::Location::ILocationService>()->StopListening();
}

void AppHost::NotifyScreenPropertiesChanged(const std::shared_ptr<Eegeo::Rendering::ScreenProperties>& screenProperties)
{
	Eegeo_ASSERT(m_app != nullptr);
	m_app->NotifyScreenPropertiesChanged(screenProperties);
}

void AppHost::SetSharedSurface(EGLSurface sharedSurface)
{
    ASSERT_NATIVE_THREAD

    // TODO:
    //m_pAndroidPlatformAbstractionModule->UpdateSurface(sharedSurface);
}

void AppHost::SetViewportOffset(float x, float y)
{
    ASSERT_NATIVE_THREAD

    m_inputHandler.SetViewportOffset(x, y);
}

void AppHost::HandleTouchInputEvent(const Eegeo::Android::Input::TouchInputEvent& event)
{
    ASSERT_NATIVE_THREAD

    m_wiring->Resolve<Eegeo::Android::Input::AndroidInputProcessor>()->HandleInput(event);
}

void AppHost::Update(float dt)
{
    ASSERT_NATIVE_THREAD

    if(m_isPaused || m_app == nullptr)
    {
        return;
    }

    GetMessageBus().FlushToNative();

    m_app->Update(dt);

    // TODO:
    //m_pModalBackgroundNativeViewModule->Update(dt);

    //if(m_pApp->IsLoadingScreenComplete() && !m_requestedApplicationInitialiseViewState)
    //{
    //    m_requestedApplicationInitialiseViewState = true;
    //    DispatchRevealUiMessageToUiThreadFromNativeThread();
    //}
}

void AppHost::Draw(float dt)
{
    ASSERT_NATIVE_THREAD

    if(m_isPaused || m_app == nullptr)
    {
        return;
    }

    m_app->Draw(dt);
    //TODO:
    m_wiring->Resolve<Eegeo::Android::Input::AndroidInputProcessor>()->Update(dt);
}

void AppHost::HandleApplicationUiCreatedOnNativeThread()
{
    ASSERT_NATIVE_THREAD
	Eegeo_TTY("HandleApplicationUiCreatedOnNativeThread begin");

	m_wiring->ResolveNativeLeaves();
	m_wiring->ResolveModules();
	m_app = m_wiring->BuildMobileExampleApp();

	Eegeo_TTY("ResolveLeaf native Thread");
	m_uiCreatedMessageReceivedOnNativeThread = true;
	PublishNetworkConnectivityStateToUIThread();
	Eegeo_TTY("HandleApplicationUiCreatedOnNativeThread end");
}

void AppHost::PublishNetworkConnectivityStateToUIThread()
{
    // Network validation runs before UI is constructed and so it is not notified and assumed there is no connection: MPLY-6584
    // The state should be passed on opening the UI view, but currently this is done from the UI thread. Pin Creation UI probably needs a refactor.

    ASSERT_NATIVE_THREAD
    
    const bool connectionIsValid = m_wiring->Resolve<Eegeo::Web::WebConnectivityValidator>()->IsValid();
    GetMessageBus().Publish(ExampleApp::Net::ConnectivityChangedViewMessage(connectionIsValid));

    //TODO:
    //const Eegeo::Web::WebConnectivityValidator& webConnectivityValidator = m_pApp->World().GetWebConnectivityValidator();
    //const bool connectionIsValid = webConnectivityValidator.IsValid();
    //m_messageBus.Publish(ExampleApp::Net::ConnectivityChangedViewMessage(connectionIsValid));
}

void AppHost::DispatchRevealUiMessageToUiThreadFromNativeThread()
{
    ASSERT_NATIVE_THREAD

    AndroidSafeNativeThreadAttachment attached(m_nativeState);
    JNIEnv* env = attached.envForThread;
    jmethodID dispatchRevealUiMessageToUiThreadFromNativeThread = env->GetMethodID(m_nativeState.activityClass, "dispatchRevealUiMessageToUiThreadFromNativeThread", "(J)V");
    env->CallVoidMethod(m_nativeState.activity, dispatchRevealUiMessageToUiThreadFromNativeThread, (jlong)(this));
}

void AppHost::DispatchUiCreatedMessageToNativeThreadFromUiThread()
{
    ASSERT_UI_THREAD

	Eegeo_TTY("DispatchUiCreatedMessageToNativeThreadFromUiThread begin");

    AndroidSafeNativeThreadAttachment attached(m_nativeState);
    JNIEnv* env = attached.envForThread;
    jmethodID dispatchUiCreatedMessageToNativeThreadFromUiThread = env->GetMethodID(m_nativeState.activityClass, "dispatchUiCreatedMessageToNativeThreadFromUiThread", "(J)V");
    env->CallVoidMethod(m_nativeState.activity, dispatchUiCreatedMessageToNativeThreadFromUiThread, (jlong)(this));

    Eegeo_TTY("DispatchUiCreatedMessageToNativeThreadFromUiThread end");
}

void AppHost::RevealUiFromUiThread()
{
    ASSERT_UI_THREAD

    m_app->InitialiseApplicationViewState(m_wiring->GetContainer());
}

void AppHost::CreateUiFromUiThread()
{
    ASSERT_UI_THREAD

    Eegeo_ASSERT(!m_registeredUIModules);
    CreateApplicationViewModulesFromUiThread();
    DispatchUiCreatedMessageToNativeThreadFromUiThread();
}

void AppHost::UpdateUiViewsFromUiThread(float dt)
{
    ASSERT_UI_THREAD

	if (m_app != nullptr)
	{
		GetMessageBus().FlushToUi();
	}

    if(!m_resolvedUIModules)
    {
        CreateUiFromUiThread();
    }
}

void AppHost::DestroyUiFromUiThread()
{
    ASSERT_UI_THREAD

    DestroyApplicationViewModulesFromUiThread();
}

void AppHost::CreateApplicationViewModulesFromUiThread()
{
	Eegeo_TTY("CreateApplicationViewModulesFromUiThread begin");
    ASSERT_UI_THREAD
    m_wiring->ResolveUiLeaves();
	m_resolvedUIModules = true;

    //TODO:



    //ExampleApp::ViewControllerUpdater::View::IViewControllerUpdaterModel& viewControllerUpdaterModel = m_pViewControllerUpdaterModule->GetViewControllerUpdaterModel();

    //viewControllerUpdaterModel.AddUpdateableObject(m_pSettingsMenuViewModule->GetMenuController());
    //viewControllerUpdaterModel.AddUpdateableObject(m_pSearchMenuViewModule->GetMenuController());

    SetTouchExclusivity();
	Eegeo_TTY("CreateApplicationViewModulesFromUiThread end");

    //GetMessageBus().SubscribeUi(m_userInteractionEnabledChangedHandler);
}

void AppHost::DestroyApplicationViewModulesFromUiThread()
{
    ASSERT_UI_THREAD

    if(m_resolvedUIModules)
    {
    	GetMessageBus().UnsubscribeUi(m_userInteractionEnabledChangedHandler);
    }
    m_resolvedUIModules = false;
    m_registeredUIModules = false;
}

void AppHost::SetTouchExclusivity()
{
	ASSERT_UI_THREAD

	AndroidSafeNativeThreadAttachment attached(m_nativeState);
	JNIEnv* env = attached.envForThread;

	const std::string methodName = "setTouchExclusivity";
	jmethodID setTouchExclusivityMethod = env->GetMethodID(m_nativeState.activityClass, methodName.c_str(), "()V");
	env->CallVoidMethod(m_nativeState.activity, setTouchExclusivityMethod);
}


void AppHost::HandleUserInteractionEnabledChanged(const ExampleApp::UserInteraction::UserInteractionEnabledChangedMessage& message)
{
	ASSERT_UI_THREAD

	AndroidSafeNativeThreadAttachment attached(m_nativeState);
	JNIEnv* env = attached.envForThread;

	const std::string methodName = "setTouchEnabled";
	jmethodID touchEnabledMethod = env->GetMethodID(m_nativeState.activityClass, methodName.c_str(), "(Z)V");
	env->CallVoidMethod(m_nativeState.activity, touchEnabledMethod, message.IsEnabled());
}

ExampleApp::ExampleAppMessaging::TMessageBus& AppHost::GetMessageBus()
{
	Eegeo_ASSERT(m_resolvedUIModules);
    return *(m_wiring->Resolve<ExampleApp::ExampleAppMessaging::TMessageBus>());
}

