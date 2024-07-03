/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include "videoOutputPort.hpp"
#include <iostream>
#include "videoOutputPortConfig.hpp"
#include "audioOutputPortConfig.hpp"

using namespace std;

namespace device {

	const char * VideoOutputPort::kPropertyResolution = ".resolution";


VideoOutputPort & VideoOutputPort::getInstance(int id)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return VideoOutputPortConfig::getInstance().getPort(id);
}
VideoOutputPort & VideoOutputPort::getInstance(const std::string &name)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return VideoOutputPortConfig::getInstance().getPort(name);
}
VideoOutputPort::VideoOutputPort(const int type, const int index, const int id, int audioPortId, const std::string &resolution) :
										_type(type), _index(index), _id(id),
										_handle(-1), _enabled(true), _contentProtected(false),
										_displayConnected(false), _aPortId(audioPortId),
										_defaultResolution(resolution),
										_resolution(resolution),
										_display(*this)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}
bool VideoOutputPort::setScartParameter(const std::string parameter, const std::string value)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return false;
}
int VideoOutputPort::getVideoEOTF() const
{
    printf("\nDUMMY %s\n", __FUNCTION__);
    return 0;
}
int VideoOutputPort::getMatrixCoefficients() const
{
	printf("\nDUMMY %s\n", __FUNCTION__);
        return 0;
}
int VideoOutputPort::getColorDepth() const
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return 0;
}
int VideoOutputPort::getColorSpace() const
{
	printf("\nDUMMY %s\n", __FUNCTION__);
        return 0;
}
int VideoOutputPort::getQuantizationRange() const
{
	printf("\nDUMMY %s\n", __FUNCTION__);
        return 0;
}
void VideoOutputPort::getCurrentOutputSettings(int &videoEOTF, int &matrixCoefficients, int &colorSpace, int &colorDepth, int &quantizationRange) const
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}
VideoOutputPort::~VideoOutputPort()
{
}
const VideoOutputPortType & VideoOutputPort::getType() const
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return VideoOutputPortConfig::getInstance().getPortType(_type);
}
AudioOutputPort &VideoOutputPort::getAudioOutputPort()
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return AudioOutputPortConfig::getInstance().getPort(_aPortId);
}
const VideoResolution & VideoOutputPort::getResolution() 
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return VideoResolution::getInstance(_resolution,true);
}
const VideoResolution & VideoOutputPort::getDefaultResolution() const
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return VideoResolution::getInstance(_defaultResolution,true);
}
const VideoOutputPort::Display &VideoOutputPort::getDisplay() 
{
	printf("\nDUMMY %s\n", __FUNCTION__);
	return _display;
}
bool VideoOutputPort::isDisplayConnected() const
{
	printf("\nDUMMY %s\n", __FUNCTION__);
        return false;
}
bool VideoOutputPort::isEnabled() const
{
	printf("\nDUMMY %s\n", __FUNCTION__);
        return false;
}
bool VideoOutputPort::isActive() const
{
	printf("\nDUMMY %s\n", __FUNCTION__);
        return false;
}
bool VideoOutputPort::isDynamicResolutionSupported() const
{
	printf("\nDUMMY %s\n", __FUNCTION__);
        return false;
}
void VideoOutputPort::setResolution(const std::string &resolutionName, bool persist/* = true*/, bool isIgnoreEdid/* = false*/)
{
	printf("\nDUMMY %s\n", __FUNCTION__);
}
void VideoOutputPort::setPreferredColorDepth(const unsigned int colordepth, bool persist/* = true*/)
{
	 printf("\nDUMMY %s\n", __FUNCTION__);
}
unsigned int VideoOutputPort::getPreferredColorDepth(bool persist/* = true*/)
{
	 printf("\nDUMMY %s\n", __FUNCTION__);
	 return 0;
}
void VideoOutputPort::getColorDepthCapabilities (unsigned int *capabilities) const
{
	 printf("\nDUMMY %s\n", __FUNCTION__);
}
void VideoOutputPort::setDisplayConnected(const bool connected)
{
	 printf("\nDUMMY %s\n", __FUNCTION__);
}
bool VideoOutputPort::isContentProtected() const
{
	 printf("\nDUMMY %s\n", __FUNCTION__);
	 return false;
}
void VideoOutputPort::enable()
{
	 printf("\nDUMMY %s\n", __FUNCTION__);
}
void VideoOutputPort::disable()
{
	 printf("\nDUMMY %s\n", __FUNCTION__);
}
VideoOutputPort::Display::Display(VideoOutputPort &vPort) :
                                                          _productCode(0), _serialNumber(0),
                                                          _manufacturerYear(0), _manufacturerWeek(0),
                                                          _hdmiDeviceType(true), _isSurroundCapable(false),
                                                          _isDeviceRepeater(false), _aspectRatio(0),
                                                          _physicalAddressA(1), _physicalAddressB(0),
                                                          _physicalAddressC(0), _physicalAddressD(0),
                                                          _handle(-1)
{
	 printf("\nDUMMY %s\n", __FUNCTION__);
}
bool VideoOutputPort::Display::hasSurround(void) const
{
	 printf("\nDUMMY %s\n", __FUNCTION__);
	 return false;
}
int VideoOutputPort::Display::getSurroundMode(void) const
{
	 printf("\nDUMMY %s\n", __FUNCTION__);
	 return 0;
}
void VideoOutputPort::Display::getEDIDBytes(std::vector<uint8_t> &edid) const
{
	 printf("\nDUMMY %s\n", __FUNCTION__);
}
VideoOutputPort::Display::~Display()
{
}
int VideoOutputPort::getHDCPStatus() 
{
	 printf("\nDUMMY %s\n", __FUNCTION__);
	 return 0;
}
int VideoOutputPort::getHDCPProtocol()
{
	 printf("\nDUMMY %s\n", __FUNCTION__);
	 return 0;
}
int VideoOutputPort::getHDCPReceiverProtocol()
{
	 printf("\nDUMMY %s\n", __FUNCTION__);
	 return 0;
}

}
