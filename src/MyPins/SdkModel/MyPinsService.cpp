// Copyright eeGeo Ltd (2012-2015), All Rights Reserved

#include "MyPinsService.h"
#include "IMyPinsRepository.h"
#include "PinRepository.h"
#include "IWorldPinsFactory.h"
#include "Pin.h"
#include "MyPinsFileIO.h"
#include "IWebLoadRequestFactory.h"
#include "IWebLoadRequest.h"
#include "ImageUploadData.h"
#include "IMenuOptionsModel.h"
#include "MyPinMenuOption.h"
#include "IWorldPinsService.h"
#include "WorldPinFocusData.h"
#include "IMyPinSelectionHandlerFactory.h"
#include "MyPinSelectionHandler.h"

#include <string>
#include <sstream>

namespace ExampleApp
{
    namespace MyPins
    {
        namespace SdkModel
        {
            MyPinsService::MyPinsService(IMyPinsRepository& myPinsRepository,
                                         MyPinsFileIO& myPinsFileIO,
                                         IMyPinSelectionHandlerFactory& myPinSelectionHandlerFactory,
                                         WorldPins::SdkModel::IWorldPinsService& worldPinsService,
                                         Eegeo::Web::IWebLoadRequestFactory& webLoadRequestFactory)
                : m_myPinsRepository(myPinsRepository)
                , m_myPinsFileIO(myPinsFileIO)
                , m_myPinSelectionHandlerFactory(myPinSelectionHandlerFactory)
                , m_worldPinsService(worldPinsService)
                , m_webLoadRequestFactory(webLoadRequestFactory)
                , m_lastIdUsed(m_myPinsFileIO.GetLastIdWrittenToDisk())
                , m_webRequestCompleteCallback(this, &MyPinsService::WebRequestCompleteCallback)
            {
                std::vector<MyPinModel*> pinModels;
                m_myPinsFileIO.LoadPinModelsFromDisk(pinModels);

                for (std::vector<MyPinModel*>::iterator it = pinModels.begin(); it != pinModels.end(); ++it)
                {
                    m_myPinsRepository.AddItem(*it);
                    AddPinToMap(*it);
                }
            }

            void MyPinsService::AddPinToMap(MyPinModel* pMyPinModel)
            {
                WorldPins::SdkModel::WorldPinFocusData worldPinFocusData(pMyPinModel->GetTitle(), pMyPinModel->GetDescription());
                MyPinSelectionHandler* selectionHandler = m_myPinSelectionHandlerFactory.CreateMyPinSelectionHandler(*pMyPinModel);

                const int myPinIconIndex = 9;

                WorldPins::SdkModel::WorldPinItemModel* worldPinItemModel = m_worldPinsService.AddPin(selectionHandler, worldPinFocusData, pMyPinModel->GetLatLong(), myPinIconIndex);
                m_myPinToWorldPinMap.insert(std::make_pair(pMyPinModel, worldPinItemModel));
            }

            void MyPinsService::RemovePinWithId(const int myPinId)
            {
                for (TMyPinToWorldPinMap::iterator it = m_myPinToWorldPinMap.begin(); it != m_myPinToWorldPinMap.end(); ++it)
                {
                    MyPinModel* pinModel = it->first;
                    WorldPins::SdkModel::WorldPinItemModel* worldPinItemModel = it->second;

                    if (pinModel->Identifier() == myPinId)
                    {
                        if (!pinModel->GetImagePath().empty())
                        {
                            m_myPinsFileIO.DeleteImageFromDisk(pinModel->GetImagePath());
                        }

                        m_worldPinsService.RemovePin(worldPinItemModel);

                        m_myPinsRepository.RemoveItem(pinModel);
                        m_myPinToWorldPinMap.erase(it);

                        break;
                    }
                }
            }

            void MyPinsService::SavePin(const std::string& title,
                                        const std::string& description,
                                        const Eegeo::Space::LatLong& latLong,
                                        Byte* imageData,
                                        size_t imageSize,
                                        bool shouldShare)
            {
                unsigned int idForThisPin = ++m_lastIdUsed;
                std::string imagePath = "";

                if (imageData)
                {
                    m_myPinsFileIO.TryCacheImageToDisk(imageData, imageSize, idForThisPin, imagePath);
                }

                MyPinModel *pinModel = Eegeo_NEW(MyPinModel)(idForThisPin,
                                       title,
                                       description,
                                       imagePath,
                                       latLong);

                m_myPinsFileIO.SavePinModelToDisk(*pinModel);

                if (shouldShare)
                {
                    SubmitPinToWebService(*pinModel);
                }

                m_myPinsRepository.AddItem(pinModel);
                AddPinToMap(pinModel);
            }

            void MyPinsService::SubmitPinToWebService(const ExampleApp::MyPins::SdkModel::MyPinModel &myPinModel)
            {
                Eegeo_TTY("Here is where you submit pin to web service\n");
            }

            void MyPinsService::WebRequestCompleteCallback(Eegeo::Web::IWebLoadRequest& webLoadRequest)
            {
                Eegeo_TTY("Web Request Completed, code: %d\n", webLoadRequest.HttpStatusCode());
            }
        }
    }
}