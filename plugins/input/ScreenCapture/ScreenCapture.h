
#ifndef OMNI_INPUT_ScreenCapture_H_
#define OMNI_INPUT_ScreenCapture_H_

#include <QImage>
#include <QOpenGLTexture>

#include <omni/visual/ContextBoundPtr.h>
#include <omni/input/Framebuffer.h>

#include "ScreenCaptureBackend.h"

namespace omni
{
	namespace input
	{
		class ScreenCapture : public QObject, public Framebuffer
		{
			Q_OBJECT
				Q_PLUGIN_METADATA(IID OMNI_INPUT_INTERFACE_IID)
				Q_INTERFACES(omni::input::Interface)
				OMNI_PLUGIN_INFO("Screen capture", "Copyright (C) 2026")

		public:
			OMNI_REGISTER_CLASS(Factory, ScreenCapture)

				enum class CaptureMode {
				Monitor,
				Window,
				Region
			};

			struct CaptureRect {
				int x, y;
				int w, h;
			};

			ScreenCapture();
			~ScreenCapture() final = default;

			GLuint textureId() const;

			void update() override;
			QSize size() const override;
			QWidget* widget() override;

			void setMode(CaptureMode);
			CaptureMode mode() const;

			void setFlipFrame(bool);
			bool flipFrame() const;

		private:
			void activate() override;
			void deactivate() override;
			void timerEvent(QTimerEvent*) override;

			CaptureMode mode_ = CaptureMode::Monitor;

			CaptureRect win_ = { 0, 0, 0, 0 };
			CaptureRect display_ = { 0, 0, 0, 0 };
			CaptureRect region_ = { 0, 0, 0, 0 };

			CaptureRect current_;

			bool flipFrame_ = true;

			int timerId_ = 0;

			ScreenCaptureBackend capture_;
			static QImage frameImage_;

			static ContextBoundPtr<QOpenGLTexture> frameTexture_;
			static ContextBoundPtr<QOpenGLShaderProgram> simpleShader_;
		};
	}
}

OMNI_DECL_ENUM_STREAM_OPERATORS(omni::input::ScreenCapture::CaptureMode);

#endif /* OMNI_INPUT_SCREENCAPTURE_H_ */

