// Copyright eeGeo Ltd (2012-2014), All Rights Reserved

#include <string>
#include <algorithm>
#include "MyPinDetailsView.h"
#include "MathFunc.h"
#include "UIColors.h"
#include "ImageHelpers.h"
#include "IconResources.h"
#include "StringHelpers.h"
#include "MyPinDetailsViewController.h"

@implementation MyPinDetailsView

- (id)initWithController:(MyPinDetailsViewController*)controller
{
	self = [super init];

	if(self)
	{
		m_pController = controller;
		self.alpha = 0.f;
		m_stateChangeAnimationTimeSeconds = 0.2;
        m_labelsSectionWidth = 0.f;
        m_maxContentSize = 0.f;

		self.pShadowContainer = [[[UIView alloc] initWithFrame:CGRectMake(0, 0, 0, 0)] autorelease];
		self.pShadowContainer.backgroundColor = ExampleApp::Helpers::ColorPalette::BlackTone;
		self.pShadowContainer.alpha = 0.1f;
		[self addSubview: self.pShadowContainer];

		self.pControlContainer = [[[UIView alloc] initWithFrame:CGRectMake(0, 0, 0, 0)] autorelease];
		self.pControlContainer.backgroundColor = ExampleApp::Helpers::ColorPalette::MainHudColor;
		[self addSubview: self.pControlContainer];

		self.pCloseButtonContainer = [[[UIView alloc] initWithFrame:CGRectMake(0, 0, 0, 0)] autorelease];
		self.pCloseButtonContainer.backgroundColor = ExampleApp::Helpers::ColorPalette::GoldTone;
		[self.pControlContainer addSubview: self.pCloseButtonContainer];

		self.pCloseButton = [[[UIButton alloc] initWithFrame:CGRectMake(0, 0, 0, 0)] autorelease];
		[self.pCloseButton setBackgroundImage:[UIImage imageNamed:@"button_close_off.png"] forState:UIControlStateNormal];
        [self.pCloseButton addTarget:self action:@selector(onCloseButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
		[self.pCloseButtonContainer addSubview: self.pCloseButton];
        
        self.pRemovePinButton = [[[UIButton alloc] initWithFrame:CGRectMake(0, 0, 0, 0)] autorelease];
        [self.pRemovePinButton setBackgroundImage:[UIImage imageNamed:@"button_remove_pin.png"] forState:UIControlStateNormal];
        [self.pRemovePinButton addTarget:self action:@selector(onRemovePinButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
        [self.pCloseButtonContainer addSubview: self.pRemovePinButton];

		self.pContentContainer = [[[UIView alloc] initWithFrame:CGRectMake(0, 0, 0, 0)] autorelease];
		self.pContentContainer.backgroundColor = ExampleApp::Helpers::ColorPalette::WhiteTone;
		[self.pControlContainer addSubview: self.pContentContainer];

		self.pHeadlineContainer = [[[UIView alloc] initWithFrame:CGRectMake(0, 0, 0, 0)] autorelease];
		self.pHeadlineContainer.backgroundColor = ExampleApp::Helpers::ColorPalette::MainHudColor;
		[self.pControlContainer addSubview: self.pHeadlineContainer];

		self.pTitleLabel = [self createLabel :ExampleApp::Helpers::ColorPalette::MainHudColor :ExampleApp::Helpers::ColorPalette::WhiteTone];
		self.pTitleLabel.textColor = ExampleApp::Helpers::ColorPalette::GoldTone;
		[self.pHeadlineContainer addSubview: self.pTitleLabel];
        
        self.pLabelsContainer = [[[UIScrollView alloc] initWithFrame:CGRectMake(0, 0, 0, 0)] autorelease];
        self.pLabelsContainer.backgroundColor = ExampleApp::Helpers::ColorPalette::WhiteTone;
        [self.pContentContainer addSubview: self.pLabelsContainer];
        
		self.pDescriptionHeaderContainer = [[[UIView alloc] initWithFrame:CGRectMake(0, 0, 0, 0)] autorelease];
		self.pDescriptionHeaderContainer.backgroundColor = ExampleApp::Helpers::ColorPalette::GoldTone;
		[self.pLabelsContainer addSubview: self.pDescriptionHeaderContainer];

		self.pDescriptionHeaderLabel = [self createLabel :ExampleApp::Helpers::ColorPalette::WhiteTone :ExampleApp::Helpers::ColorPalette::GoldTone];
		[self.pDescriptionHeaderContainer addSubview: self.pDescriptionHeaderLabel];

		self.pDescriptionContent = [self createLabel :ExampleApp::Helpers::ColorPalette::MainHudColor :ExampleApp::Helpers::ColorPalette::WhiteTone];
		self.pDescriptionContent.textColor = ExampleApp::Helpers::ColorPalette::DarkGreyTone;
		[self.pLabelsContainer addSubview: self.pDescriptionContent];

		self.pImageHeaderContainer = [[[UIView alloc] initWithFrame:CGRectMake(0, 0, 0, 0)] autorelease];
		self.pImageHeaderContainer.backgroundColor = ExampleApp::Helpers::ColorPalette::GoldTone;

		[self.pLabelsContainer addSubview: self.pImageHeaderContainer];

		self.pImageHeaderLabel = [self createLabel :ExampleApp::Helpers::ColorPalette::WhiteTone :ExampleApp::Helpers::ColorPalette::GoldTone];
		[self.pImageHeaderContainer addSubview: self.pImageHeaderLabel];

        self.pImageContent = [[[UIImageView alloc] initWithFrame: CGRectMake(0, 0, 0, 0)] autorelease];
        self.pImageContent.image = [UIImage imageNamed: @"image_blank.png"];

		[self.pLabelsContainer addSubview: self.pImageContent];
	}

	return self;
}

- (void)dealloc
{
    [self.pRemovePinButton removeFromSuperview];
    [self.pRemovePinButton release];
    
	[self.pCloseButton removeFromSuperview];
	[self.pCloseButton release];

	[self.pCloseButtonContainer removeFromSuperview];
	[self.pCloseButtonContainer release];

	[self.pShadowContainer removeFromSuperview];
	[self.pShadowContainer release];

	[self.pControlContainer removeFromSuperview];
	[self.pControlContainer release];

	[self.pHeadlineContainer removeFromSuperview];
	[self.pHeadlineContainer release];

	[self.pLabelsContainer removeFromSuperview];
	[self.pLabelsContainer release];

	[self.pContentContainer removeFromSuperview];
	[self.pContentContainer release];

	[self.pTitleLabel removeFromSuperview];
	[self.pTitleLabel release];

	[self.pDescriptionHeaderLabel removeFromSuperview];
	[self.pDescriptionHeaderLabel release];

	[self.pDescriptionHeaderContainer removeFromSuperview];
	[self.pDescriptionHeaderContainer release];

	[self.pDescriptionContent removeFromSuperview];
	[self.pDescriptionContent release];

	[self.pImageHeaderLabel removeFromSuperview];
	[self.pImageHeaderLabel release];

	[self.pImageHeaderContainer removeFromSuperview];
	[self.pImageHeaderContainer release];

	[self.pImageContent removeFromSuperview];
	[self.pImageContent release];

	[self removeFromSuperview];
	[super dealloc];
}

- (void)layoutSubviews
{
	self.alpha = 0.f;

	const float boundsWidth = self.superview.bounds.size.width;
	const float boundsHeight = self.superview.bounds.size.height;
	const bool useFullScreenSize = (boundsHeight < 600.f || boundsWidth < 400.f);
	const float boundsOccupyMultiplier = useFullScreenSize ? 0.9f : 0.5f;
	const float mainWindowWidth = boundsWidth * boundsOccupyMultiplier;
	const float mainWindowHeight = boundsHeight * boundsOccupyMultiplier;
	const float mainWindowX = (boundsWidth * 0.5f) - (mainWindowWidth * 0.5f);
	const float mainWindowY = (boundsHeight * 0.5f) - (mainWindowHeight * 0.5f);

	self.frame = CGRectMake(mainWindowX,
	                        mainWindowY,
	                        mainWindowWidth,
	                        mainWindowHeight);

	self.pControlContainer.frame = CGRectMake(0.f,
	                               0.f,
	                               mainWindowWidth,
	                               mainWindowHeight);

	self.pShadowContainer.frame = CGRectMake(2.f,
	                              2.f,
	                              mainWindowWidth,
	                              mainWindowHeight);
	const float headlineHeight = 50.f;
	const float closeButtonSectionHeight = 80.f;
	const float closeButtonSectionOffsetY = mainWindowHeight - closeButtonSectionHeight;
	const float contentSectionHeight = mainWindowHeight - (closeButtonSectionHeight + headlineHeight);
	const float shadowHeight = 10.f;

	self.pHeadlineContainer.frame = CGRectMake(0.f,
	                                0.f,
	                                mainWindowWidth,
	                                headlineHeight);

	ExampleApp::Helpers::ImageHelpers::AddPngImageToParentView(self.pHeadlineContainer, "shadow_03", 0.f, headlineHeight, mainWindowWidth, shadowHeight);


	self.pContentContainer.frame = CGRectMake(0.f,
	                               headlineHeight,
	                               mainWindowWidth,
	                               contentSectionHeight);

	const float labelsSectionOffsetX = 12.f;
	m_labelsSectionWidth = mainWindowWidth - (2.f * labelsSectionOffsetX);

    self.pLabelsContainer.frame = CGRectMake(labelsSectionOffsetX,
                                             0.f,
                                             m_labelsSectionWidth,
                                             contentSectionHeight);


	self.pCloseButtonContainer.frame = CGRectMake(0.f,
	                                   closeButtonSectionOffsetY,
	                                   mainWindowWidth,
	                                   closeButtonSectionHeight);

	ExampleApp::Helpers::ImageHelpers::AddPngImageToParentView(self.pContentContainer, "shadow_03", 0.f, contentSectionHeight, mainWindowWidth, shadowHeight);

	self.pCloseButton.frame = CGRectMake(mainWindowWidth - closeButtonSectionHeight,
	                                     0.f,
	                                     closeButtonSectionHeight,
	                                     closeButtonSectionHeight);
    
    self.pRemovePinButton.frame = CGRectMake(0.f,
                                             0.f,
                                             closeButtonSectionHeight,
                                             closeButtonSectionHeight);

	const float titlePadding = 10.0f;
	self.pTitleLabel.frame = CGRectMake(titlePadding,
	                                    0.f,
	                                    mainWindowWidth - titlePadding,
	                                    headlineHeight);
	self.pTitleLabel.font = [UIFont systemFontOfSize:24.0f];
	self.pTitleLabel.text = @"";

	const float headerLabelHeight = 20.f;
	const float labelYSpacing = 8.f;
	float currentLabelY = labelYSpacing;

	const float headerTextPadding = 3.0f;
	self.pDescriptionHeaderContainer.frame = CGRectMake(0.f, currentLabelY, m_labelsSectionWidth, headerLabelHeight + 2 * headerTextPadding);

	self.pDescriptionHeaderLabel.frame = CGRectMake(headerTextPadding, headerTextPadding, m_labelsSectionWidth - headerTextPadding, headerLabelHeight);
	self.pDescriptionHeaderLabel.text = @"Description";
	currentLabelY += labelYSpacing + self.pDescriptionHeaderContainer.frame.size.height;

	self.pDescriptionContent.frame = CGRectMake(headerTextPadding, currentLabelY, m_labelsSectionWidth - headerTextPadding, 60.f);
	self.pDescriptionContent.text = @"";
//	self.pDescriptionContent.adjustsFontSizeToFitWidth = NO;
    self.pDescriptionContent.lineBreakMode = NSLineBreakByTruncatingTail;
	currentLabelY += labelYSpacing + self.pDescriptionContent.frame.size.height;

	self.pImageHeaderContainer.frame = CGRectMake(0.f, currentLabelY, m_labelsSectionWidth, headerLabelHeight + 2 * headerTextPadding);

	self.pImageHeaderLabel.frame = CGRectMake(headerTextPadding, headerTextPadding, m_labelsSectionWidth - headerTextPadding, headerLabelHeight);
	self.pImageHeaderLabel.text = @"Image";
	currentLabelY += labelYSpacing + self.pImageHeaderContainer.frame.size.height;

	self.pImageContent.frame = CGRectMake(headerTextPadding, currentLabelY, m_labelsSectionWidth - headerTextPadding, m_labelsSectionWidth * 0.75f);
    currentLabelY += labelYSpacing + self.pImageContent.frame.size.height;
    
	[self.pLabelsContainer setContentSize:CGSizeMake(m_labelsSectionWidth, currentLabelY)];
    m_maxContentSize = currentLabelY;
}

- (void) setContent:(const ExampleApp::MyPins::MyPinModel*)pModel
{
	self.pTitleLabel.text = [NSString stringWithUTF8String:pModel->GetTitle().c_str()];

	self.pDescriptionHeaderContainer.hidden = true;
	self.pDescriptionContent.hidden = true;
	self.pImageHeaderContainer.hidden = true;
	self.pImageContent.hidden = true;

    float scrollContentHeight = 60.f;
    
    std::string descriptionString = pModel->GetDescription().c_str();
    const int maxCharactersInLine = 40;
    int numberOfLines = descriptionString.length() / maxCharactersInLine;
//    NSLog(@"Number of lines: %d\n", numberOfLines);
    
    self.pDescriptionHeaderContainer.hidden = false;
    self.pDescriptionContent.hidden = false;
    self.pDescriptionContent.numberOfLines = numberOfLines;
    self.pDescriptionContent.text = [NSString stringWithUTF8String: pModel->GetDescription().c_str()];
//    self.pDescriptionContent.backgroundColor = [UIColor redColor];

//    [self.pDescriptionContent sizeToFit];


	if(!pModel->GetImagePath().empty())
	{
        NSArray* libraryPaths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
        NSString* libraryDirectory = [libraryPaths objectAtIndex:0];
        NSString* imageFilename = [NSString stringWithUTF8String: pModel->GetImagePath().c_str()];
        NSString* fullPathToImage  = [libraryDirectory stringByAppendingPathComponent: imageFilename];
        
        self.pImageContent.image = [UIImage imageWithContentsOfFile: fullPathToImage];
		self.pImageHeaderContainer.hidden = false;
		self.pImageContent.hidden = false;
        scrollContentHeight = m_maxContentSize;
	}

    self.pLabelsContainer.contentSize = CGSizeMake(m_labelsSectionWidth, scrollContentHeight);
	[self.pLabelsContainer setContentOffset:CGPointMake(0,0) animated:NO];
}

- (void) setFullyActive
{
	if(self.alpha == 1.f)
	{
		return;
	}

	[self animateToAlpha:1.f];
}

- (void) setFullyInactive
{
	if(self.alpha == 0.f)
	{
		return;
	}

	[self animateToAlpha:0.f];
}

- (void) setActiveStateToIntermediateValue:(float)openState
{
	self.alpha = openState;
}

- (BOOL)consumesTouch:(UITouch *)touch
{
	return self.alpha > 0.f;
}

- (void) animateToAlpha:(float)alpha
{
	[UIView animateWithDuration:m_stateChangeAnimationTimeSeconds
	 animations:^
	{
		self.alpha = alpha;
	}];
}

- (UILabel*) createLabel:(UIColor*)textColor :(UIColor*)backgroundColor
{
	UILabel* pLabel = [[[UILabel alloc] initWithFrame:CGRectMake(0, 0, 0, 0)] autorelease];
	pLabel.textColor = textColor;
	pLabel.backgroundColor = backgroundColor;
	pLabel.textAlignment = NSTextAlignmentLeft;
	return pLabel;
}

- (void) onCloseButtonPressed:(UIButton *)sender
{
    [m_pController handleCloseButtonPressed];
}

- (void) onRemovePinButtonPressed:(UIButton *)sender
{
    UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Remove Pin"
                                                    message:@"Are you sure you want to remove this pin?"
                                                   delegate:self
                                          cancelButtonTitle:@"No, keep it"
                                          otherButtonTitles:@"Yes, delete it", nil];
    
    [alert show];
    [alert release];
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if (buttonIndex == 1)
    {
        [m_pController handleRemovePinButtonPressed];
    }
}

@end