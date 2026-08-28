
#include "ScreenCapture.h"

#include <omni/visual/util.h>
#include <omni/visual/Shader.h>
#include <omni/visual/Rectangle.h>

#include "ScreenCaptureWidget.h"

namespace omni
{
	namespace input
	{
		QImage ScreenCapture::frameImage_;
		ContextBoundPtr<QOpenGLTexture> ScreenCapture::frameTexture_;
		ContextBoundPtr<QOpenGLShaderProgram> ScreenCapture::simpleShader_;

		ScreenCapture::ScreenCapture()
		{
		}

		GLuint ScreenCapture::textureId() const
		{
			return !frameTexture_ ? 0 : frameTexture_->textureId();
		}

		void ScreenCapture::activate()
		{
			update();

			int _timerId = startTimer(16);
			if (!timerId_)
				timerId_ = _timerId;
		}

		void ScreenCapture::deactivate()
		{
			if (timerId_)
			{
				killTimer(timerId_);
				timerId_ = 0;
			}

			frameTexture_.reset();
			simpleShader_.reset();
		}

		void ScreenCapture::timerEvent(QTimerEvent*)
		{
			update();
			triggerUpdateCallbacks();
		}

		void ScreenCapture::update()
		{
			if (!capture_.capture(frameImage_))
				return;

			bool _allocate = !frameTexture_ || !simpleShader_;
			if (frameImage_.size() != size()) {
				display_.w = frameImage_.size().width();
				display_.h = frameImage_.size().height();
			}

			if (_allocate) {
				primaryContextSwitch([&](QOpenGLFunctions& _)
				{
					setupFramebuffer(size());
					frameTexture_.reset(new QOpenGLTexture(frameImage_.rgbSwapped().mirrored()));
					initShader(simpleShader_, "ScreenCaptureImage");
				});
			}

			visual::draw_on_framebuffer(framebuffer(),
				[&](QOpenGLFunctions& _) // Projection Operation
			{
				QMatrix4x4 _m;
				_m.ortho(-0.5, 0.5, -0.5, 0.5, -1.0, 1.0);
				glMultMatrixf(_m.constData());
			},
				[&](QOpenGLFunctions& _) // Model View Operation
			{
				visual::useShader(*simpleShader_, [&](visual::UniformHandler& _h)
				{
					_h.uniform("texture_size", QVector2D(size().width(), size().height()));
					_h.texUniform("texture", textureId());
					visual::Rectangle::draw();
				});
			});

			triggerUpdateCallbacks();
		}

		QSize ScreenCapture::size() const
		{
			switch (mode_)
			{
			case CaptureMode::Monitor:
				return display_.w > 0 && display_.h > 0
					? QSize(display_.w, display_.h)
					: QSize(0, 0);

			case CaptureMode::Window:
				return QSize(win_.w, win_.h);

			case CaptureMode::Region:
				return QSize(region_.w, region_.h);
			}

			return QSize(0, 0);
		}

		QWidget* ScreenCapture::widget()
		{
			return new omni::ui::input::ScreenCapture(this);
		}

		void ScreenCapture::setMode(CaptureMode _mode)
		{
			mode_ = _mode;
			update();
		}

		ScreenCapture::CaptureMode ScreenCapture::mode() const
		{
			return mode_;
		}

		void ScreenCapture::setFlipFrame(bool _flipFrame)
		{
			flipFrame_ = _flipFrame;
		}

		bool ScreenCapture::flipFrame() const
		{
			return flipFrame_;
		}
	}
}
