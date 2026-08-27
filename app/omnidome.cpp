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


#include "MainWindow.h"

#include "Application.h"
#include <QDir>
#include <QFile>

#include <QCommandLineParser>

using namespace omni;

class CommandLineParser {
public:
    void parse(QApplication const& app) {
        const auto args = app.arguments();

        for (int i = 1; i < args.size(); ++i) {
            const QString& arg = args[i];

            if (arg.startsWith("--")) {
                const QString key = arg.mid(2);

                // Option with a following value
                if (i + 1 < args.size() && !args[i + 1].startsWith("--")) {
                    keyValues_[key] = args[++i];
                }
                else {
                    // Boolean flag
                    keyValues_[key] = "true";
                }
            }
        }
    }

    QString value(QString const& key) const {
        auto it = keyValues_.find(key);
        return it != keyValues_.end() ? it->second : QString();
    }

    bool has(QString const& key) const {
        return keyValues_.count(key) != 0;
    }

private:
    std::map<QString, QString> keyValues_;
};

int main(int ac, char *av[])
{
  // This line is absolutely mandatory for being able to have multiple
  // QOpenGLWidgets in different windows!!!
  QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

  QSurfaceFormat _format;

//  _format.setVersion(3,3);
  _format.setProfile(QSurfaceFormat::CompatibilityProfile);
#ifdef QT_DEBUG
  _format.setOption(QSurfaceFormat::DebugContext);
#endif
  QSurfaceFormat::setDefaultFormat(_format);


  omni::ui::Application _a(ac, av);

  CommandLineParser parser;
  parser.parse(_a);

  /// Command line parser is only available in debug mode
#ifdef QT_DEBUG
//  if (!parser.value("stylesheet").isEmpty()) {
//    qDebug() << parser.value("stylesheet");
//    _a.setStyleSheetFile(parser.value("stylesheet"));
//  }
#endif // ifdef DEBUG

  QString profile = parser.value("profile");
  bool live = parser.value("live").isEmpty()? false: true;
  bool hidden = parser.value("hide").isEmpty() ? false : true;

  ui::MainWindow _w;
  _w.move(QApplication::primaryScreen()->geometry().topLeft());

  // Load mapping session from given commandline argument when in release mode
  if (!profile.isEmpty())
      _w.openProjection(profile);

  if (live)
      _w.startLiveMode();

  if (hidden)
      _w.hide();
  else
      _w.show();

#if !defined (Q_OS_WIN)
  Q_INIT_RESOURCE(libomni);
#endif

  return _a.exec();
}
