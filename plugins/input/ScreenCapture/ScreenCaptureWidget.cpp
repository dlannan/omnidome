/* Copyright (c) 2014-2015 "Omnidome" by Michael Winkelmann
 * Dome Mapping Projection Software (http://omnido.me).
 * Omnidome was created by Michael Winkelmann aka Wilston Oreo (@WilstonOreo)
 *
 * This file is part of Omnidome.
 *
 * Omnidome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScreenCaptureWidget.h"

#include <QComboBox>

namespace omni {
    namespace ui {
        namespace input {
            ScreenCapture::ScreenCapture(omni::input::ScreenCapture* _input,
                QWidget* _parent) :
                ParameterWidget(_parent),
                input_(_input) {
                setup();
            }

            ScreenCapture::~ScreenCapture() {

            }

            void ScreenCapture::setMode(int _mode) {
                input_->setMode(
                  util::intToEnum<omni::input::ScreenCapture::CaptureMode>(_mode));
                preview_->update();
                emit inputChanged();
            }

            void ScreenCapture::setFlipFrame(bool _flipFrame) {
                input_->setFlipFrame(_flipFrame);
                preview_->update();
                emit inputChanged();
            }

            void ScreenCapture::triggerUpdate() {
              preview_->triggerUpdate();
            }

            void ScreenCapture::setup() {
                QLayout *_layout = new QVBoxLayout;
                _layout->setSpacing(2);
                _layout->setContentsMargins(0, 0, 0, 0);
                _layout->setSizeConstraint(QLayout::SetMaximumSize);
                setLayout(_layout);

                preview_.reset(new TestInputPreview(input_));
                _layout->addWidget(preview_.get());

                auto* _boxMode = new QComboBox();
                _boxMode->addItem("Monitor");
                _boxMode->addItem("Window");
                _boxMode->addItem("Region");
                connect(_boxMode,SIGNAL(currentIndexChanged(int)),this,SLOT(setMode(int)));
                _layout->addWidget(_boxMode);

                auto* _chkFlipText = addCheckBox("Flip text",input_->flipFrame());
                connect(_chkFlipText,SIGNAL(clicked(bool)),this,SLOT(setFlipFrame(bool)));
                connect(preview_.get(),SIGNAL(inputChanged()),this,SIGNAL(inputChanged()));
            }
        }
    }
}
