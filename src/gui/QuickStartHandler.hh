/*
 * Copyright (C) 2022 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef IGNITION_GAZEBO_GUI_QUICKSTARTHANDLER_HH_
#define IGNITION_GAZEBO_GUI_QUICKSTARTHANDLER_HH_

#include <QtCore>
#include <QDesktopServices>
#include <string>

#include "ignition/gazebo/EntityComponentManager.hh"
#include "ignition/gazebo/Export.hh"

namespace ignition
{
namespace gazebo
{
// Inline bracket to help doxygen filtering.
inline namespace IGNITION_GAZEBO_VERSION_NAMESPACE {
namespace gui
{
/// \brief Class for handling quick start dialog
class QuickStartHandler : public QObject
{
  Q_OBJECT

  /// \brief Get worlds path
  /// \return worlds directory path
  public: Q_INVOKABLE QString WorldsPath() const;

  /// \brief Get the distribution name
  /// \return Distribution name, such as 'Citadel'
  public: Q_INVOKABLE QString Distribution() const;

  /// \brief Get Gazebo version
  /// \return gazebo version
  public: Q_INVOKABLE QString GazeboVersion() const;

  /// \brief Set starting world
  /// \param[in] _url Url to the world file.
  public: Q_INVOKABLE void SetStartingWorld(const QString &_url);

  /// \brief Get starting world url from GUI.
  /// \return World url
  public: std::string StartingWorld() const;

  /// \brief Get default config of the dialog.
  /// \return config as string
  public: std::string Config() const;

  /// \brief Set the flag to show quick start menu again.
  /// \param[in] _showQuickStartOpts True to show.
  public: Q_INVOKABLE void SetShowDefaultQuickStartOpts(
      const bool _showQuickStartOpts);

  /// \brief Show quick start menu option.
  public: bool ShowDefaultQuickStartOpts() const;

  /// \brief Show the quick start menu again.
  private: bool showDefaultQuickStartOpts{true};

  /// \brief Installed worlds path.
  private: std::string worldsPath{IGN_GAZEBO_WORLD_INSTALL_DIR};

  /// \brief Get starting world url.
  private: std::string startingWorld{""};

};
}
}
}
}
#endif