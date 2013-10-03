/*
* 
* Copyright (c) 2013, Ban the Rewind
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or 
* without modification, are permitted provided that the following 
* conditions are met:
* 
* Redistributions of source code must retain the above copyright 
* notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright 
* notice, this list of conditions and the following disclaimer in 
* the documentation and/or other materials provided with the 
* distribution.
* 
* Neither the name of the Ban the Rewind nor the names of its 
* contributors may be used to endorse or promote products 
* derived from this software without specific prior written 
* permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
*/

#pragma once
#if defined( _DEBUG )
#pragma comment( lib, "comsuppwd.lib" )
#else
#pragma comment( lib, "comsuppw.lib" )
#endif
#pragma comment( lib, "wbemuuid.lib" )

#include "cinder/Cinder.h"
#include "cinder/Exception.h"
#include "cinder/Matrix.h"
#include "cinder/Quaternion.h"
#include "cinder/Surface.h"
#include <functional>
#include <list>
#include <map>
#include "ole2.h"
#include "KinectWrapperLib.h"
#include <vector>

//! Kinect SDK wrapper for Cinder
namespace MsKinect
{
class Device;
class DeviceOptions;
typedef NUI_SKELETON_BONE_ROTATION		BoneRotation;
typedef NUI_IMAGE_RESOLUTION			ImageResolution;
typedef NUI_SKELETON_POSITION_INDEX		JointName;
typedef KINECT_SKELETON_SELECTION_MODE	SkeletonSelectionMode;
typedef std::shared_ptr<Device>			DeviceRef;

//////////////////////////////////////////////////////////////////////////////////////////////

class DepthProcessOptions
{
public:
	DepthProcessOptions();

	/*! Enables binary mode where background is black and users 
		are white. Set \a inverted to true to reverse. Enabling 
		binary mode also enables background removal. */
	DepthProcessOptions&				enableBinary( bool enable = true, bool inverted = false );
	//! Normalizes non-user pixels.
	DepthProcessOptions&				enableRemoveBackground( bool enable = true );
	//! Colorizes user pixels.
	DepthProcessOptions&				enableUserColor( bool enable = true );

	//! Returns true if image is black and white.
	bool								isBinaryEnabled() const;
	//! Returns true if black and white image is inverted.
	bool								isBinaryInverted() const;
	//! Returns true if background removal is enabled.
	bool								isRemoveBackgroundEnabled() const;
	//! Returns true if user colorization is enabled.
	bool								isUserColorEnabled() const;
protected:
	bool								mBinary;
	bool								mBinaryInverted;
	bool								mRemoveBackground;
	bool								mUserColor;
};

//////////////////////////////////////////////////////////////////////////////////////////////

//! Counts the number of users in \a depth.
size_t									calcNumUsersFromDepth( const ci::Channel16u& depth );
//! Creates a surface with colorized users from \a depth.
ci::Surface16u							depthChannelToSurface( const ci::Channel16u& depth, 
															  const DepthProcessOptions& depthProcessOptions = DepthProcessOptions() );
//! Returns number of Kinect devices.
size_t									getDeviceCount();
//! Returns use color for user ID \a id.
ci::Colorf								getUserColor( uint32_t id );
//! Returns pixel location of color position in depth image.
ci::Vec2i								mapColorCoordToDepth( const ci::Vec2i& v, const ci::Channel16u& depth, 
															 ImageResolution colorResolution, ImageResolution depthResolution );
//! Returns pixel location of skeleton position in color image. Requires depth resolution.s
ci::Vec2i								mapSkeletonCoordToColor( const ci::Vec3f& v, const ci::Channel16u& depth, 
																ImageResolution colorResolution, ImageResolution depthResolution );
//! Returns pixel location of skeleton position in depth image.
ci::Vec2i								mapSkeletonCoordToDepth( const ci::Vec3f& v, ImageResolution depthResolution );
//! Returns user ID for pixel at \a coord in \a depth. 0 is no user.
uint16_t								userIdFromDepthCoord( const ci::Channel16u& depth, const ci::Vec2i& v );

//////////////////////////////////////////////////////////////////////////////////////////////

/*! Skeleton smoothing enumeration. Smoother transform improves skeleton accuracy, 
	but increases latency. */
enum : uint_fast8_t
{
	TRANSFORM_NONE, TRANSFORM_DEFAULT, TRANSFORM_SMOOTH, TRANSFORM_VERY_SMOOTH, TRANSFORM_MAX
} typedef								SkeletonTransform;

//////////////////////////////////////////////////////////////////////////////////////////////

class Bone
{
public:
	//! Returns rotation of the bone relative to camera coordinates.
	const ci::Quatf&					getAbsoluteRotation() const;
	//! Returns rotation matrix of the bone relative to camera coordinates.
	const ci::Matrix44f&				getAbsoluteRotationMatrix() const;
	//! Returns index of end joint.
	JointName							getEndJoint() const;
	//! Returns position of the bone's start joint.
	const ci::Vec3f&					getPosition() const;
	//! Returns rotation of the bone relative to the parent bone.
	const ci::Quatf&					getRotation() const;
	//! Returns rotation matrix of the bone relative to the parent bone.
	const ci::Matrix44f&				getRotationMatrix() const;
	//! Returns index of start joint.
	JointName							getStartJoint() const;
private:
	Bone( const Vector4& position, const _NUI_SKELETON_BONE_ORIENTATION& bone );
	ci::Matrix44f						mAbsRotMat;
	ci::Quatf							mAbsRotQuat;
	JointName							mJointEnd;
	JointName							mJointStart;
	ci::Vec3f							mPosition;
	ci::Matrix44f						mRotMat;
	ci::Quatf							mRotQuat;

	friend class						Device;
};
typedef std::map<JointName, Bone>		Skeleton;

//////////////////////////////////////////////////////////////////////////////////////////////

class DeviceOptions
{
public:
	//! Default settings
	DeviceOptions();

	//! Returns resolution of color image.
	ImageResolution						getColorResolution() const; 
	//! Returns size of color image.
	const ci::Vec2i&					getColorSize() const; 
	//! Returns resolution of depth image.
	ImageResolution						getDepthResolution() const; 
	//! Returns size of depth image.
	const ci::Vec2i&					getDepthSize() const; 
	//! Returns unique ID for this device.
	const std::string&					getDeviceId() const;
	//! Returns 0-index for this device.
	int32_t								getDeviceIndex() const;
	//! Returns skeleton selection mode for this device.
	SkeletonSelectionMode				getSkeletonSelectionMode() const;
	//! Returns skeleton transform for this device.
	SkeletonTransform					getSkeletonTransform() const;

	//! Returns true if color video stream is enabled.
	bool								isColorEnabled() const;
	//! Returns true if depth tracking is enabled.
	bool								isDepthEnabled() const;
	//! Returns true if near mode is enabled.
	bool								isNearModeEnabled() const;
	//! Returns true if seated mode is enabled.
	bool								isSeatedModeEnabled() const;
	//! Returns true if user tracking is enabled.
	bool								isUserTrackingEnabled() const;

	//! Enables color stream.
	DeviceOptions&						enableColor( bool enable = true );
	//! Enables depth tracking.
	DeviceOptions&						enableDepth( bool enable = true );
	//! Enables near mode. Kinect for Windows only.
	DeviceOptions&						enableNearMode( bool enable = true );
	//! Enables seated mode. Kinect for Windows only.
	DeviceOptions&						enableSeatedMode( bool enable = true );
	/*! Enables user tracking. */
	DeviceOptions&						enableUserTracking( bool enable = true );
	
	//! Sets resolution of color image.
	DeviceOptions&						setColorResolution( const ImageResolution& resolution = ImageResolution::NUI_IMAGE_RESOLUTION_640x480 );
	//! Sets resolution of depth image.
	DeviceOptions&						setDepthResolution( const ImageResolution& resolution = ImageResolution::NUI_IMAGE_RESOLUTION_320x240 ); 
	//! Starts device with this unique ID.
	DeviceOptions&						setDeviceId( const std::string& id = "" ); 
	//! Starts device with this 0-index.
	DeviceOptions&						setDeviceIndex( int32_t index = 0 ); 
	//! Set skeleton selection mode to \a mode.
	DeviceOptions& 						setSkeletonSelectionMode( SkeletonSelectionMode mode );
	//! Set skeleton transform to \a tranform.
	DeviceOptions& 						setSkeletonTransform( SkeletonTransform tranform );
protected:
	bool								mEnabledColor;
	bool								mEnabledDepth;
	bool								mEnabledNearMode;
	bool								mEnabledSeatedMode;
	bool								mEnabledUserTracking;

	SkeletonSelectionMode				mSkeletonSelectionMode;
	SkeletonTransform					mSkeletonTransform;
	
	ImageResolution						mColorResolution;
	ci::Vec2i							mColorSize;
	ImageResolution						mDepthResolution;
	ci::Vec2i							mDepthSize;

	std::string							mDeviceId;
	int32_t								mDeviceIndex;
};
//////////////////////////////////////////////////////////////////////////////////////////////

/*! Class representing Kinect frame data. A frame only contains data 
	for enabled features (e.g., skeletons are empty if skeleton tracking 
	is disabled). */
class Frame
{
public:
	Frame();

	//! Returns color surface for this frame.
	const ci::Surface8u&				getColorSurface() const;
	//! Returns depth surface for this frame.
	const ci::Channel16u&				getDepthChannel() const;
	//! Returns unique identifier for the sensor that generated the frame.
	const std::string&					getDeviceId() const;
	//! Returns unique, sequential frame ID.
	long long							getFrameId() const;
	//! Returns skeletons for this frame.
	const std::vector<Skeleton>&		getSkeletons() const;
protected:
	Frame( long long frameId, const std::string& deviceId, 
		const ci::Surface8u& color, const ci::Channel16u& depth, const std::vector<Skeleton>& skeletons );

	ci::Surface8u						mColorSurface;
	ci::Channel16u						mDepthChannel;
	std::string							mDeviceId;
	long long							mFrameId;
	std::vector<Skeleton>				mSkeletons;

	friend class						Device;
};

//////////////////////////////////////////////////////////////////////////////////////////////

// Kinect sensor interface
class Device
{
public:
	//! Maximum number of devices supported by the Kinect SDK.
	static const int32_t				MAXIMUM_DEVICE_COUNT	= 8;
	//! Maximum device tilt angle in positive or negative degrees.
	static const int32_t				MAXIMUM_TILT_ANGLE		= 28;

	~Device();
	
	//! Creates pointer to instance of Kinect
	static DeviceRef					create();		
	
	//! Start capturing using settings specified in \a deviceOptions .
	void								start( const DeviceOptions& deviceOptions = DeviceOptions() );
	//! Stop capture.
	void								stop();
	
	//! Enables verbose error reporting in debug console. Default is true.
	void								enableVerbose( bool enable = true );

	//! Returns depth value as 0.0 - 1.0 float for pixel at \a pos.
	float								getDepthAt( const ci::Vec2i& v ) const;
	//! Returns options object for this device.
	const DeviceOptions&				getDeviceOptions() const;
	//! Returns accelerometer reading.
	ci::Quatf							getOrientation() const;
	//! Returns current device angle in degrees between -28 and 28.
	int32_t								getTilt();
	//! Returns number of tracked users. Depth resolution must be no more than 320x240 with user tracking enabled.
	int32_t								getUserCount();

	//! Returns true is actively capturing.
	bool								isCapturing() const;

	//! Sets device angle to \a degrees. Default is 0.
	void								setTilt( int32_t degrees = 0 );

	//! Sets frame event handler. Signature is void( Frame ).
	template<typename T, typename Y> 
	inline void							connectEventHandler( T eventHandler, Y *obj )
	{
		connectEventHandler( std::bind( eventHandler, obj, std::placeholders::_1 ) );
	}
	//! Sets frame event handler. Signature is void( Frame ).
	void								connectEventHandler( const std::function<void ( Frame )>& eventHandler );
protected:
	static const int32_t				WAIT_TIME = 100;

	Device();
	
	void								init( bool reset = false );
	virtual void						update();

	std::function<void ( Frame frame )>	mEventHandler;

	DeviceOptions						mDeviceOptions;

	void*								mKinect;
	INuiSensor*							mNuiSensor;

	uint8_t*							mBufferColor;
	uint8_t*							mBufferDepth;
	ci::Channel16u						mChannelDepth;
	KINECT_IMAGE_FRAME_FORMAT			mFormatColor;
	KINECT_IMAGE_FRAME_FORMAT			mFormatDepth;
	long long							mFrameId;
	std::vector<Skeleton>				mSkeletons;
	ci::Surface8u						mSurfaceColor;
	
	bool								mCapture;
	bool								mIsSkeletonDevice;
	bool								mVerbose;

	double								mTiltRequestTime;
	
	void								errorNui( long hr );
	void								statusKinect( KINECT_SENSOR_STATUS status );
	std::string							wcharToString( wchar_t* v );
	
	friend void __stdcall				deviceStatus( long hr, const WCHAR* instanceName, const WCHAR* deviceId, void* data );

	//////////////////////////////////////////////////////////////////////////////////////////////

public:
	class Exception : public ci::Exception
	{
	public:
		const char* what() const throw();
	protected:
		char			mMessage[ 2048 ];
		friend class	Device;
	};

	//! Exception representing failure to create device.
	class ExcDeviceCreate : public Exception 
	{
	public:
		ExcDeviceCreate( long hr, const std::string& id ) throw();
	};

	//! Exception representing failure to initialize device.
	class ExcDeviceInit : public Exception 
	{
	public:
		ExcDeviceInit( long hr, const std::string& id ) throw();
	};

	//! Exception representing attempt to create device with invalid index or ID.
	class ExcDeviceInvalid : public Exception 
	{
	public:
		ExcDeviceInvalid( long hr, const std::string& id ) throw();
	};

	//! Exception representing failure to open color stream.
	class ExcOpenStreamColor : public Exception
	{
	public:
		ExcOpenStreamColor( long hr ) throw();
	};

	//! Exception representing failure to open depth stream.
	class ExcOpenStreamDepth : public Exception
	{
	public:
		ExcOpenStreamDepth( long hr ) throw();
	};

	//! Exception representing failure to enable skeleton tracking.
	class ExcSkeletonTrackingEnable : public Exception
	{
	public:
		ExcSkeletonTrackingEnable( long hr ) throw();
	};
};
}
 