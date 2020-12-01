#pragma once

#include <hyperion/GrabberWrapper.h>
#include <grabber/MFGrabber.h>

class MFWrapper : public GrabberWrapper
{
	Q_OBJECT

public:
	MFWrapper(const QString & device,
			const unsigned grabWidth,
			const unsigned grabHeight,
			const unsigned fps,
			const unsigned input,			
			PixelFormat pixelFormat,			
			const QString & configurationPath );
	~MFWrapper() override;

	bool getSignalDetectionEnable() const;
	bool getCecDetectionEnable() const;
	int getHdrToneMappingEnabled();

public slots:
	bool start() override;
	void stop() override;

	void setSignalThreshold(double redSignalThreshold, double greenSignalThreshold, double blueSignalThreshold, int noSignalCounterThreshold);
	void setCropping(unsigned cropLeft, unsigned cropRight, unsigned cropTop, unsigned cropBottom) override;
	void setSignalDetectionOffset(double verticalMin, double horizontalMin, double verticalMax, double horizontalMax);
	void setSignalDetectionEnable(bool enable);
	void setCecDetectionEnable(bool enable);
	void setDeviceVideoStandard(const QString& device);
	void handleSettingsUpdate(settings::type type, const QJsonDocument& config) override;		
	
	///
	/// @brief set software decimation (v4l2)
	///
	void setFpsSoftwareDecimation(int decimation);
	
	///
	/// @brief enable HDR to SDR tone mapping (v4l2)
	///
	void setHdrToneMappingEnabled(int mode);	
	
	void setEncoding(QString enc);
	
	void setBrightnessContrastSaturationHue(int brightness, int contrast, int saturation, int hue);
	
private slots:
	void newFrame(const Image<ColorRgb> & image);
	void readError(const char* err);

	void action() override;

private:
	/// The V4L2 grabber
	MFGrabber _grabber;
};
