
#include "ScreenCapture.h"

#include <omni/visual/util.h>
#include <omni/visual/Shader.h>
#include <omni/visual/Rectangle.h>

#include "ScreenCaptureWidget.h"
#include <GL/glcorearb.h>

#include <QGuiApplication>
#include <QScreen>


namespace omni
{
	namespace input
	{
		ContextBoundPtr<QOpenGLShaderProgram> ScreenCapture::shader_;

		ScreenCapture::ScreenCapture()
		{
			mode_ = CaptureMode::Monitor;
			sourceX_ = 0.0f;
			sourceY_ = 0.0f;
			sourceWidth_ = 1.0f;
			sourceHeight_ = 1.0f;
			windowFound_ = nullptr;
		}

		ScreenCapture::~ScreenCapture()
		{
		}

		GLuint ScreenCapture::textureId() const
		{
			return  textureId_;
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
		}

		void ScreenCapture::timerEvent(QTimerEvent*)
		{
			update();
			triggerUpdateCallbacks();
		}

		void ScreenCapture::update()
		{
			if (!QOpenGLContext::currentContext()) return;
			if (!capture_.IsInitialized()) {
				capture_.Init(monitorSelect_);
			}

			using namespace visual;

			if (capture_.size() != size() || !shader_)
			{
				primaryContextSwitch([&](QOpenGLFunctions& _)
					{
						setupFramebuffer(capture_.size());
						initShader(shader_, "SimpleShader");
					});
				display_.w = capture_.size().width();
				display_.h = capture_.size().height();
			}

			if (!capture_.isCapturing()) return;
			if (!framebuffer() || !framebuffer()->isValid()) return;

			if (mode_ == CaptureMode::Window && windowFound_)
				checkWindow();

			textureId_ = framebuffer()->texture();

			draw_on_framebuffer(framebuffer(), [&](QOpenGLFunctions&) {
				QMatrix4x4 m;
				m.ortho(-0.5, 0.5, -0.5, 0.5, -1.0, 1.0);
				glMultMatrixf(m.constData());

				}, [&](QOpenGLFunctions& _) {
					capture_.CaptureTexture();

					useShader(*shader_, [&](UniformHandler& h) {
						h.texUniform("texture", capture_.textureId());

						h.uniform("sourceRect",
							QVector4D(sourceX_, sourceY_,
								sourceWidth_, sourceHeight_));

						Rectangle::draw();
						});
					});


			triggerUpdateCallbacks();
		}

		QSize ScreenCapture::size() const
		{
			return display_.w > 0 && display_.h > 0
				? QSize(display_.w, display_.h)
				: QSize(0, 0);
		}

		/// Serialize image path to stream
		void ScreenCapture::toPropertyMap(PropertyMap& _map) const
		{
			Framebuffer::toPropertyMap(_map);
		}

		/// Deserialize from stream and load image
		void ScreenCapture::fromPropertyMap(PropertyMap const& _map)
		{
			Framebuffer::fromPropertyMap(_map);
		}

		QWidget* ScreenCapture::widget()
		{
			return new omni::ui::input::ScreenCapture(this);
		}

		void ScreenCapture::setMode(CaptureMode _mode)
		{
			mode_ = _mode;
			switch (mode_) {
				case CaptureMode::Monitor:
					sourceX_ = 0.0f;
					sourceY_ = 0.0f;
					sourceWidth_ = 1.0f;
					sourceHeight_ = 1.0f;
					windowFound_ = nullptr;
					break;
				case CaptureMode::Window:
					break;
				case CaptureMode::Region:
					break;
			}
			update();
		}

		ScreenCapture::CaptureMode ScreenCapture::mode() const
		{
			return mode_;
		}

		void ScreenCapture::setWindowName(QString name) {
			windowName_ = name;
		}
		QString ScreenCapture::getWindowName() const {
			return windowName_;
		}

		void ScreenCapture::setRegionSize(QSize sz)
		{
			regionSize_ = sz;
		}
		QSize ScreenCapture::getRegionSize() {
			return regionSize_;
		}

		void ScreenCapture::setRegionPos(QSize pos)
		{
			regionPos_ = pos;
		}

		QSize ScreenCapture::getRegionPos()
		{
			return regionPos_;
		}

		void ScreenCapture::setMonitor(int id)
		{
			int maxmonitors = capture_.getMonitorCount();
			if (maxmonitors == 0) return;

			id = std::max(0, std::min(id, maxmonitors - 1));
			if (id != monitorSelect_) {
				monitorSelect_ = id;
				capture_.SetInitialized(false);
			}
			update();
		}

		int ScreenCapture::getMonitor()
		{
			return monitorSelect_;
		}

		void ScreenCapture::checkWindow()
		{
			RECT rect{};
			GetClientRect(windowFound_, &rect);
			POINT topLeft{ rect.left, rect.top };
			ClientToScreen(windowFound_, &topLeft);

			// Calc UVs based on desktop!
			sourceX_ = topLeft.x / float(capture_.size().width());
			sourceY_ = topLeft.y / float(capture_.size().height());
			sourceWidth_ = (rect.right - rect.left) / float(capture_.size().width());
			sourceHeight_ = (rect.bottom - rect.top) / float(capture_.size().height());
		}

		// Gets the window wnd.. 
		void ScreenCapture::findWindow()
		{
			QByteArray nameUtf8 = windowName_.toUtf8();

			HWND hwnd = FindWindowA(nullptr, nameUtf8.constData());
			if (hwnd) {
				windowFound_ = hwnd;
				return;
			}

			sourceX_ = 0.0f;
			sourceY_ = 0.0f;
			sourceWidth_ = 1.0f;
			sourceHeight_ = 1.0f;
		}

		void ScreenCapture::regionChanged()
		{
			// Calc UVs based on desktop!
			sourceX_ = regionPos_.width() / float(capture_.size().width());
			sourceY_ = regionPos_.height() / float(capture_.size().height());
			sourceWidth_ = regionSize_.width() / float(capture_.size().width());
			sourceHeight_ = regionSize_.height() / float(capture_.size().height());
		}
	}
}
