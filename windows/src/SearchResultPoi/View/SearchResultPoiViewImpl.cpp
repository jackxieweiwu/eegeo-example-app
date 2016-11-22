// Copyright eeGeo Ltd (2012-2015), All Rights Reserved

#include "SearchResultPoiController.h"
#include "SearchResultPoiViewImpl.h"
#include "WindowsAppThreadAssertionMacros.h"
#include "SearchResultModelCLI.h"
#include "ReflectionHelpers.h"
#include "SearchVendorNames.h"

using namespace ExampleApp::Helpers::ReflectionHelpers;

using namespace System;
using namespace System::Reflection;

namespace ExampleApp
{
    namespace SearchResultPoi
    {
        namespace View
        {
            namespace
            {
                const std::string VendorViewClassNames[] = {
                    "ExampleAppWPF.YelpSearchResultsPoiView",
                    "ExampleAppWPF.eeGeoSearchResultsPoiView",
                    "ExampleAppWPF.GeoNamesSearchResultsPoiView"
                };
            }

            SearchResultPoiViewImpl::SearchResultPoiViewImpl(const std::shared_ptr<WindowsNativeState>& nativeState)
                : m_nativeState(nativeState)
                , m_currentVendor(-1)
            {
                for (int i = 0; i < SearchVendors::Num; ++i)
                {
                    m_uiViewClass[i] = GetTypeFromEntryAssembly(Helpers::ReflectionHelpers::ConvertUTF8ToManagedString(VendorViewClassNames[i]));
                    ConstructorInfo^ ctor = m_uiViewClass[i]->GetConstructor(CreateTypes(IntPtr::typeid));
                    m_uiView[i] = ctor->Invoke(CreateObjects(gcnew IntPtr(this)));

                    DisplayPoiInfo[i].SetupMethod(m_uiViewClass[i], m_uiView[i], "DisplayPoiInfo");
                    DismissPoiInfo[i].SetupMethod(m_uiViewClass[i], m_uiView[i], "DismissPoiInfo");
                    UpdateImageData[i].SetupMethod(m_uiViewClass[i], m_uiView[i], "UpdateImageData");
                }
            }

            SearchResultPoiViewImpl::~SearchResultPoiViewImpl()
            {
            }

            void SearchResultPoiViewImpl::Show(const Search::SdkModel::SearchResultModel& model, bool isPinned)
            {
                m_model = model;
                CreateVendorSpecificPoiView(m_model.GetVendor());

				DisplayPoiInfo[m_currentVendor](gcnew SearchResultModelCLI(m_model), isPinned);
            }

            void SearchResultPoiViewImpl::Hide()
            {
                DismissPoiInfo[m_currentVendor]();
            }

            void SearchResultPoiViewImpl::UpdateImage(const std::string& url, bool hasImage, const std::vector<unsigned char>* pImageBytes)
            {
                if (!hasImage || pImageBytes->empty())
                {
                    return;
                }
                
                array<System::Byte>^ imageDataArray = gcnew array<System::Byte>(static_cast<int>(pImageBytes->size()));

                for (size_t i = 0; i < pImageBytes->size(); ++i)
                {
                    imageDataArray[static_cast<int>(i)] = System::Byte(pImageBytes->at(i));
                }

                UpdateImageData[m_currentVendor](gcnew System::String(url.c_str()), hasImage, imageDataArray);
            }

            void SearchResultPoiViewImpl::InsertClosedCallback(Eegeo::Helpers::ICallback0& callback)
            {
                m_closedCallbacks.AddCallback(callback);
            }

            void SearchResultPoiViewImpl::RemoveClosedCallback(Eegeo::Helpers::ICallback0& callback)
            {
                ASSERT_UI_THREAD

                m_closedCallbacks.RemoveCallback(callback);
            }

            void SearchResultPoiViewImpl::HandleCloseClicked()
            {
                m_closedCallbacks.ExecuteCallbacks();
            }

            void SearchResultPoiViewImpl::InsertTogglePinnedCallback(Eegeo::Helpers::ICallback1<Search::SdkModel::SearchResultModel>& callback)
            {
                ASSERT_UI_THREAD

                m_togglePinClickedCallbacks.AddCallback(callback);
            }

            void SearchResultPoiViewImpl::RemoveTogglePinnedCallback(Eegeo::Helpers::ICallback1<Search::SdkModel::SearchResultModel>& callback)
            {
                ASSERT_UI_THREAD

                m_togglePinClickedCallbacks.RemoveCallback(callback);
            }

            void SearchResultPoiViewImpl::HandlePinToggleClicked()
            {
                ASSERT_UI_THREAD

                m_togglePinClickedCallbacks.ExecuteCallbacks(m_model);
            }

            void SearchResultPoiViewImpl::CreateVendorSpecificPoiView(const std::string& vendor)
            {
                ASSERT_UI_THREAD

                if(vendor == Search::YelpVendorName)
                {
                    m_currentVendor = SearchVendors::Yelp;
                }
                else if(vendor == Search::GeoNamesVendorName)
                {
                    m_currentVendor = SearchVendors::GeoNames;
                }
                else if (vendor == Search::EegeoVendorName)
                {
                    m_currentVendor = SearchVendors::eeGeo;
                }
                else if (vendor == Search::ExampleTourVendorName)
                {
                    Eegeo_ASSERT(false, "Unable to creaate view instance for %s. Tour views are not yet implemented on Windows - please refer to iOS example.\n", vendor.c_str());
                    m_currentVendor = -1;
                }
                else
                {
                    Eegeo_ASSERT(false, "Unknown POI vendor %s, cannot create view instance.\n", vendor.c_str());
                    m_currentVendor = -1;
                }
            }
        }
    }
}