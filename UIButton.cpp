//******************************************************************************
//
// Copyright (c) 2015 Microsoft Corporation. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
//******************************************************************************

#include "UIButton.h"
#include "UIColor.h"
#include "UIStoryboardSegue.h"
#include "UICustomResource.h"
#include "UIRuntimeEventConnection.h"
#include "UIFont.h"
#include <assert.h>

void ConvertInsets(struct _PropertyMapper* prop, NIBWriter* writer, XIBObject* propObj, XIBObject* obj) {
    struct {
        float top, left, bottom, right;
    } EdgeInsets;

    char szName[255];

    sprintf(szName, "IBUI%sEdgeInsets.top", prop->nibName);
    EdgeInsets.top = obj->FindMember(szName)->floatValue();
    sprintf(szName, "IBUI%sEdgeInsets.left", prop->nibName);
    EdgeInsets.left = obj->FindMember(szName)->floatValue();
    sprintf(szName, "IBUI%sEdgeInsets.bottom", prop->nibName);
    EdgeInsets.bottom = obj->FindMember(szName)->floatValue();
    sprintf(szName, "IBUI%sEdgeInsets.right", prop->nibName);
    EdgeInsets.right = obj->FindMember(szName)->floatValue();

    char* dataOut = (char*)malloc(sizeof(EdgeInsets) + 1);
    dataOut[0] = 6;
    memcpy(&dataOut[1], &EdgeInsets, sizeof(EdgeInsets));
    sprintf(szName, "UI%sEdgeInsets", prop->nibName);
    obj->AddOutputMember(writer, strdup(szName), new XIBObjectDataWriter(dataOut, sizeof(EdgeInsets) + 1));
}

static XIBObject* GetButtonContent(NIBWriter* writer, XIBObject* obj, char* mode) {
    XIBObject* buttonContent = new XIBObject();
    buttonContent->_className = "UIButtonContent";
    buttonContent->_outputClassName = "UIButtonContent";
    buttonContent->_needsConversion = false;

    char szName[255];
    XIBObject* findObj;

    sprintf(szName, "IBUI%sTitle", mode);
    findObj = obj->FindMember(szName);
    if (findObj) {
        buttonContent->AddOutputMember(writer, "UITitle", findObj);
    }

    sprintf(szName, "IBUI%sImage", mode);
    findObj = obj->FindMember(szName);
    if (findObj) {
        buttonContent->AddOutputMember(writer, "UIImage", findObj);
    }

    sprintf(szName, "IBUI%sBackgroundImage", mode);
    findObj = obj->FindMember(szName);
    if (findObj) {
        buttonContent->AddOutputMember(writer, "UIBackgroundImage", findObj);
    }

    sprintf(szName, "IBUI%sTitleShadowColor", mode);
    findObj = obj->FindMember(szName);
    if (findObj) {
        buttonContent->AddOutputMember(writer, "UIShadowColor", findObj);
    }

    sprintf(szName, "IBUI%sTitleColor", mode);
    findObj = obj->FindMember(szName);
    if (findObj) {
        buttonContent->AddOutputMember(writer, "UITitleColor", findObj);
    } else {
        if (strcmp(mode, "Normal") != 0 && buttonContent->_outputMembers.size() > 0) {
            // findObj = obj->FindMember("IBUINormalTitleColor");
            // buttonContent->AddOutputMember(writer, "UITitleColor", findObj);
        } else if (strcmp(mode, "Normal") == 0) {
            UIColor* color = new UIColor(4, 4, 0.0f, 0.0f, 0.0f, 0.0f, "whiteColor");
            buttonContent->AddOutputMember(writer, "UITitleColor", color->CreateObject(writer));
        }
    }

    return buttonContent;
}

static XIBObject* GetButtonContentStoryboard(NIBWriter* writer, XIBObject* obj, char* mode) {
    XIBObject* buttonContent = new XIBObject();
    buttonContent->_className = "UIButtonContent";
    buttonContent->_outputClassName = "UIButtonContent";
    buttonContent->_needsConversion = false;

    obj = obj->FindMemberAndHandle(mode);
    if (!obj) {
        return buttonContent;
    }

    if (obj->getAttrib("image") != NULL) {
        UICustomResource* image = new UICustomResource();
        image->_imageName = obj->getAttrAndHandle("image");
        buttonContent->AddOutputMember(writer, "UIImage", image);
    }

    if (obj->getAttrib("backgroundImage") != NULL) {
        UICustomResource* image = new UICustomResource();
        image->_imageName = obj->getAttrAndHandle("backgroundImage");
        buttonContent->AddOutputMember(writer, "UIBackgroundImage", image);
    }

    if (obj->getAttrib("title") != NULL) {
        buttonContent->AddOutputMember(writer, "UITitle", new XIBObjectString(obj->getAttrAndHandle("title")));
    }

    if (obj->FindMember("titleShadowColor") != NULL) {
        buttonContent->AddOutputMember(writer, "UIShadowColor", obj->FindMemberAndHandle("titleShadowColor"));
    }

    if (obj->FindMember("titleColor") != NULL) {
        buttonContent->AddOutputMember(writer, "UITitleColor", obj->FindMemberAndHandle("titleColor"));
    } else if (strcmp(mode, "normal") == 0) {
        UIColor* color = new UIColor(0, 4, 0.0f, 0.47f, 0.84f, 1.0f, NULL);
        color->_isStory = true;
        buttonContent->AddOutputMember(writer, "UITitleColor", color);
    }

    return buttonContent;
}

void UIButton::WriteStatefulContent(NIBWriter* writer, XIBObject* obj) {
    XIBDictionary* contentDict = new XIBDictionary();
    contentDict->_className = "NSMutableDictionary";

    XIBObjectNumber* normalState = new XIBObjectNumber(0);
    XIBObject* normalContent = NULL;
    XIBObjectNumber* highlightedState = new XIBObjectNumber(1);
    XIBObject* highlightedContent = NULL;
    XIBObjectNumber* disabledState = new XIBObjectNumber(2);
    XIBObject* disabledContent = NULL;
    XIBObjectNumber* selectedState = new XIBObjectNumber(4);
    XIBObject* selectedContent = NULL;

    if (!_isStory) {
        normalContent = GetButtonContent(writer, obj, "Normal");
        highlightedContent = GetButtonContent(writer, obj, "Highlighted");
        disabledContent = GetButtonContent(writer, obj, "Disabled");
        selectedContent = GetButtonContent(writer, obj, "Selected");
    } else {
        normalContent = GetButtonContentStoryboard(writer, obj, "normal");
        highlightedContent = GetButtonContentStoryboard(writer, obj, "highlighted");
        disabledContent = GetButtonContentStoryboard(writer, obj, "disabled");
        selectedContent = GetButtonContentStoryboard(writer, obj, "selected");
    }

    if (normalContent->_outputMembers.size() > 0) {
        contentDict->AddObjectForKey(normalState, normalContent);
    }
    if (highlightedContent->_outputMembers.size() > 0) {
        contentDict->AddObjectForKey(highlightedState, highlightedContent);
        obj->AddOutputMember(writer, "UIAdjustsImageWhenHighlighted", new XIBObjectBool(true));
    } else {
        obj->AddOutputMember(writer, "UIAdjustsImageWhenHighlighted", new XIBObjectBool(true));
    }
    if (disabledContent->_outputMembers.size() > 0) {
        contentDict->AddObjectForKey(disabledState, disabledContent);
        obj->AddOutputMember(writer, "UIAdjustsImageWhenDisabled", new XIBObjectBool(true));
    } else {
        obj->AddOutputMember(writer, "UIAdjustsImageWhenDisabled", new XIBObjectBool(true));
    }
    if (selectedContent->_outputMembers.size() > 0) {
        contentDict->AddObjectForKey(selectedState, selectedContent);
    }

    obj->AddOutputMember(writer, "UIButtonStatefulContent", contentDict);
}

static PropertyMapper propertyMappings[] = {
    "IBUIText",
    "UIText",
    NULL,
    "IBUITextColor",
    "UITextColor",
    NULL,
    "IBUIHighlightedColor",
    "UIHighlightedColor",
    NULL,
    "IBUILineBreakMode",
    "UILineBreakMode",
    NULL,
    "IBUIContentEdgeInsets.top",
    "Content",
    ConvertInsets,
    "IBUITitleEdgeInsets.top",
    "Title",
    ConvertInsets,
    "IBUIImageEdgeInsets.top",
    "Image",
    ConvertInsets,
    "IBUITitleShadowOffset",
    "TitleShadow",
    ConvertOffset,
    "IBUIShowsTouchWhenHighlighted",
    "UIShowsTouchWhenHighlighted",
    NULL,
    "IBUIImage",
    "UIImage",
    NULL,
    "IBUIBackgroundImage",
    "UIBackgroundImage",
    NULL,
};
static const int numPropertyMappings = sizeof(propertyMappings) / sizeof(PropertyMapper);

UIButton::UIButton() {
    _buttonType = 0;
    _statefulContent = NULL;
    _font = NULL;
}

void UIButton::InitFromXIB(XIBObject* obj) {
    UIControl::InitFromXIB(obj);

    _buttonType = GetInt("IBUIButtonType", 0);
    _font = (UIFont*)obj->FindMember("IBUIFontDescription");
    if (!_font)
        _font = (UIFont*)obj->FindMember("IBUIFont");

    switch (_buttonType) {
        case 1:
            obj->_outputClassName = "UIRoundedRectButton";
            break;

        default:
            obj->_outputClassName = "UIButton";
            break;
    }
}

void UIButton::InitFromStory(XIBObject* obj) {
    UIControl::InitFromStory(obj);

    const char* type = getAttrib("buttonType");
    if (type) {
        if (strcmp(type, "roundedRect") == 0) {
            getAttrAndHandle("buttonType");
            _buttonType = 1;
        } else {
            printf("Unknown button type <%s>\n", type);
            _buttonType = 0;
        }
    }

    switch (_buttonType) {
        case 1:
            obj->_outputClassName = "UIRoundedRectButton";
            break;

        default:
            obj->_outputClassName = "UIButton";
            break;
    }
}

void UIButton::ConvertStaticMappings(NIBWriter* writer, XIBObject* obj) {
    Map(writer, obj, propertyMappings, numPropertyMappings);
    if (_buttonType != 0)
        AddInt(writer, "UIButtonType", _buttonType);
    if (_font)
        obj->AddOutputMember(writer, "UIFont", _font);

    if (_connections) {
        for (int i = 0; i < _connections->count(); i++) {
            XIBObject* curObj = _connections->objectAtIndex(i);

            if (strcmp(curObj->_className, "segue") == 0) {
                UIStoryboardSegue* segue = (UIStoryboardSegue*)curObj;

                UIRuntimeEventConnection* newEvent = new UIRuntimeEventConnection();
                newEvent->_label = "perform:";
                newEvent->_source = this;
                newEvent->_destination = segue;
                newEvent->_eventMask = 0x40;
                writer->_connections->AddMember(NULL, newEvent);
                writer->AddOutputObject(newEvent);
            }
        }
    }
    WriteStatefulContent(writer, this);
    UIControl::ConvertStaticMappings(writer, obj);
}
