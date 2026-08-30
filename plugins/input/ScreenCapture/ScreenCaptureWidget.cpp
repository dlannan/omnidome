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
#include <QLineEdit>
#include <QFormLayout>
#include <QPushButton>


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
                input_->setMode(util::intToEnum<omni::input::ScreenCapture::CaptureMode>(_mode));
                preview_->update();
                emit inputChanged();
            }

            void ScreenCapture::triggerUpdate() {
              preview_->triggerUpdate();
            }

            void ScreenCapture::setup() {
                QVBoxLayout* _layout = new QVBoxLayout;
                _layout->setSpacing(2);
                _layout->setContentsMargins(0, 0, 0, 0);
                _layout->setSizeConstraint(QLayout::SetMaximumSize);
                setLayout(_layout);

                preview_.reset(new TestInputPreview(input_));
                _layout->addWidget(preview_.get());

                auto* _monitorRow = new QWidget();
                auto* _monitorLayout = new QHBoxLayout(_monitorRow);
                auto addMonitorSpinBox = [&](const char* label, int value) {
                    _monitorLayout->addWidget(new QLabel(label));

                    auto* _spin = new QSpinBox();
                    _spin->setRange(0, 10);
                    _spin->setValue(value);
                    _spin->setMaximumWidth(60);

                    _monitorLayout->addWidget(_spin);
                    return _spin;
                    };
                auto* _monitorSelect = addMonitorSpinBox("Monitor Selct:", input_->getMonitor());

                _layout->addWidget(_monitorRow);

                auto* _boxMode = new QComboBox();
                _boxMode->addItem("Monitor");
                _boxMode->addItem("Window");
                _boxMode->addItem("Region");

                preview_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
                _boxMode->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                _monitorRow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

                connect(_boxMode, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(setMode(int)));

                _layout->addWidget(_boxMode);

                auto* _windowName = new QLineEdit();
                _windowName->setText(input_->getWindowName());

                auto* _rowWidget = new QWidget();
                auto* _windowRow = new QHBoxLayout(_rowWidget);
                _windowRow->setSpacing(2);
                _windowRow->setContentsMargins(0, 0, 0, 0);

                auto* _windowLabel = new QLabel("Window:");
                _windowRow->addWidget(_windowLabel);
                _windowRow->addWidget(_windowName);

                _layout->addWidget(_rowWidget);

                auto* _regionRow = new QWidget();
                auto* _regionLayout = new QHBoxLayout(_regionRow);
                _regionLayout->setSpacing(2);
                _regionLayout->setContentsMargins(0, 0, 0, 0);

                _regionLayout->addWidget(new QLabel("Region:"));

                auto addSpinBox = [&](const char* label, int value) {
                    _regionLayout->addWidget(new QLabel(label));

                    auto* _spin = new QSpinBox();
                    _spin->setRange(0, 99999);
                    _spin->setValue(value);
                    _spin->setMaximumWidth(60);

                    _regionLayout->addWidget(_spin);
                    return _spin;
                    };

                auto* _regionX = addSpinBox("X:", input_->getRegionPos().width());
                auto* _regionY = addSpinBox("Y:", input_->getRegionPos().height());
                auto* _regionW = addSpinBox("W:", input_->getRegionSize().width());
                auto* _regionH = addSpinBox("H:", input_->getRegionSize().height());

                _layout->addWidget(_regionRow);

                auto* _setRegion = new QPushButton("Set");
                _setRegion->setSizePolicy(
                    QSizePolicy::Fixed,
                    QSizePolicy::Fixed);

                _regionLayout->addWidget(_setRegion);

                _rowWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
                _regionRow->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

                connect(_monitorSelect, QOverload<int>::of(&QSpinBox::valueChanged),
                    this, [this](int value) {
                        input_->setMonitor(value);
                        preview_->update();
                        emit inputChanged();
                    });

                connect(_setRegion, &QPushButton::clicked,
                    this, [this, _regionX, _regionY, _regionW, _regionH]() {
                        input_->setRegionPos(QSize(_regionX->value(), _regionY->value()));
                        input_->setRegionSize(QSize(_regionW->value(),_regionH->value()));
                        input_->regionChanged();
                        preview_->update();
                        emit inputChanged();
                    });

                auto updateVisibility = [=](int mode) {
                    bool windowMode = (mode == 1);
                    bool regionMode = (mode == 2);

                    _rowWidget->setVisible(windowMode);
                    _regionRow->setVisible(regionMode);
                    };

                connect(_windowName, &QLineEdit::editingFinished,
                    this, [this, _windowName]() {
                        input_->setWindowName(_windowName->text());
                        input_->findWindow();
                        preview_->update();
                        emit inputChanged();
                    });


                connect(_boxMode,
                    QOverload<int>::of(&QComboBox::currentIndexChanged),
                    this,
                    updateVisibility);

                updateVisibility(_boxMode->currentIndex());


                connect(_windowName, &QLineEdit::editingFinished,
                    this, [this, _windowName]() {
                        input_->setWindowName(_windowName->text());
                        preview_->update();
                        emit inputChanged();
                    });

                connect(preview_.get(),SIGNAL(inputChanged()),this,SIGNAL(inputChanged()));
            }
        }
    }
}
