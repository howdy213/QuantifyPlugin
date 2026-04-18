/**
 * @file classrecord.h
 * @brief 班级记录类
 * @author howdy213
 * @date 2026-4-5
 * @version 1.4.0
 *
 * Copyright (C) 2025-2026 howdy213
 *
 * This file is part of QuantifyPlugin.
 *
 * QuantifyPlugin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QuantifyPlugin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "WECore/WPlugin/wplugin.h"
#include "WECore/WPlugin/wplugindata.h"
#include "WFile/wpath.h"
#include "logger.h"

#include "quantifyplugin.h"
using namespace we::Consts;
///
/// \brief QuantifyPlugin::QuantifyPlugin
///
QuantifyPlugin::QuantifyPlugin() {}
///
/// \brief QuantifyPlugin::~QuantifyPlugin
///
QuantifyPlugin::~QuantifyPlugin() {}
///
/// \brief QuantifyPlugin::init
/// \param msg
/// \return
///
bool QuantifyPlugin::init(WMetaData &msg) {

    PluginData::setData(qvariant_cast<WEBase *>(msg.map["WE"]));
    PluginData::setPlugin(qvariant_cast<WPlugin *>(msg.map["Plugin"]));

    if (auto plugin = qvariant_cast<WPlugin *>(msg.map["Plugin"])) {
        plugin->setMetaData(Plugin::Name, "Quantify");
        plugin->setMetaData(Plugin::Author, "howdy213");
        if(plugin->getMetaData(Plugin::Init)=="start"){
            if (widget == nullptr) {
                widget = new QuantifyDialog;
                widget->setPlugin(this);
                widget->show();
            } else
                widget->activateWindow();
        }
    }
    return true;
}
///
/// \brief QuantifyPlugin::recMsg
/// \param msg
///
void QuantifyPlugin::recMsg(WMetaData &msg) {
    if (msg.map[Data::Command] == "start") {
        if (widget == nullptr) {
            widget = new QuantifyDialog;
            widget->setPlugin(this);
            widget->show();
        } else
            widget->activateWindow();
    }
}
///
/// \brief QuantifyPlugin::deinit
/// \param msg
/// \return
///
bool QuantifyPlugin::deinit(WMetaData &msg) {
    Q_UNUSED(msg);
    return true;
}
