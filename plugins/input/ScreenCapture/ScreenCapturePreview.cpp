/* Copyright (c) 2014-2016 "Omnidome" by Michael Winkelmann
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

#include "ScreenCapturePreview.h"
#include <QMouseEvent>

namespace omni {
    namespace ui {
        TestInputPreview::TestInputPreview(QWidget *_widget) :
            InputPreview(_widget) {}

        TestInputPreview::TestInputPreview(
            input::ScreenCapture *_input, QWidget *_widget) :
            InputPreview(_input, _widget) {}

        TestInputPreview::~TestInputPreview() {}

        void TestInputPreview::mouseMoveEvent(QMouseEvent *event)
        {
            if (!input()) return;

            if (event->buttons() & Qt::LeftButton)
            {
            }
        }

        void TestInputPreview::mousePressEvent(QMouseEvent *event)
        {
            GLView::mousePressEvent(event);
            if (!input()) return;
        }

        void TestInputPreview::mouseReleaseEvent(QMouseEvent *event)
        {
            if (!input()) return;

            if (event->buttons() & Qt::LeftButton)
            {
            }
        }
    }
}
