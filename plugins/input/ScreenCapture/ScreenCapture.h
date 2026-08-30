
#ifndef OMNI_INPUT_ScreenCapture_H_
#define OMNI_INPUT_ScreenCapture_H_

#include <QImage>
#include <QOpenGLTexture>

#include <omni/visual/ContextBoundPtr.h>
#include <omni/input/Framebuffer.h>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>

#include "ScreenCaptureBackend.h"

namespace omni
{
	namespace input
	{
		class ScreenCapture : public QObject, public Framebuffer {
			Q_OBJECT
			Q_PLUGIN_METADATA(IID OMNI_INPUT_INTERFACE_IID)
			Q_INTERFACES(omni::input::Interface)
			OMNI_PLUGIN_INFO("Screen capture", "Copyright (C) 2026")

		public:
			OMNI_REGISTER_CLASS(Factory, ScreenCapture)

			ScreenCapture();
			~ScreenCapture();

			enum class CaptureMode {
				Monitor,
				Window,
				Region
			};

			struct CaptureRect {
				int x, y;
				int w, h;
			};

			GLuint textureId() const;

			void update() override;
			QSize size() const override;
			QWidget* widget() override;

			void setMode(CaptureMode);
			CaptureMode mode() const;

			void setWindowName(QString name);
			QString getWindowName() const;

			void setRegionSize(QSize sz);
			QSize getRegionSize();

			void setRegionPos(QSize pos);
			QSize getRegionPos();

			void setMonitor(int id);
			int getMonitor();

			void findWindow();
			void regionChanged();

			/// Serialize image path to property map
			void     toPropertyMap(PropertyMap&) const;

			/// Deserialize from property map and load image
			void     fromPropertyMap(PropertyMap const&);

		private:
			void activate() override;
			void deactivate() override;
			void timerEvent(QTimerEvent*) override;

			void checkWindow();

			CaptureMode mode_ = CaptureMode::Monitor;

			CaptureRect win_ = { 0, 0, 0, 0 };
			CaptureRect display_ = { 0, 0, 0, 0 };
			CaptureRect region_ = { 0, 0, 0, 0 };

			CaptureRect current_;
			int			monitorSelect_ = 0;

			QString		windowName_;
			HWND		windowFound_;
			QSize		regionSize_;
			QSize		regionPos_;

			QSize lastSize_ = { 0, 0 };

			bool flipFrame_ = true;

			int timerId_ = 0;
			GLuint textureId_ = 0;
			bool _firstRun = true;

			// UV capture area
			float sourceX_ = 0.0f;
			float sourceY_ = 0.0f;
			float sourceWidth_ = 1.0f;
			float sourceHeight_ = 1.0f;

			ScreenCaptureBackend capture_;

			static ContextBoundPtr<QOpenGLShaderProgram> shader_;
		};
	}
}

OMNI_DECL_ENUM_STREAM_OPERATORS(omni::input::ScreenCapture::CaptureMode);

#endif /* OMNI_INPUT_SCREENCAPTURE_H_ */

