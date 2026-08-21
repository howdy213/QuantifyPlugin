/**
 * @file quantifyplugin.cpp
 * @brief 插件类
 * @author howdy213
 * @date 2026-05-04
 * @version 2.0.0
 *
 * Copyright (C) 2025-2026 howdy213
 *
 * This file is part of QuantifyPlugin.
 *
 * QuantifyPlugin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QuantifyPlugin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "../LightWidget/ILightMain.h"
#include "WECore/metadata/WEvent.h"
#include "WECore/plugin/wplugin.h"
#include "WECore/plugin/wplugindata.h"
#include "WECore/plugin/wpluginmessage.h"
#include "WECore/widget/wwidgetmanager.h"

#include "quantifyplugin.h"
using namespace LightWidget::Consts;
using namespace we;
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
bool QuantifyPlugin::init(WMessage &msg) {

    PluginData::setData(qvariant_cast<WEBase *>(msg.map[Data::WEBase]));
    PluginData::setPlugin(qvariant_cast<WPlugin *>(msg.map[Data::Plugin]));
    PluginData::setWidget(msg.object);

    if (auto plugin = PPlugin) {
        if (plugin->initArg() == "start") {
            showDialog();
        }
    }

    PluginData::setData(qvariant_cast<WEBase *>(msg.map[Data::WEBase]));
    PluginData::setPlugin(qvariant_cast<WPlugin *>(msg.map[Data::Plugin]));

    auto widgetManager = PClass->widgetManager();

    QAction *action = new QAction("打开", nullptr);
    QObject::connect(action, &QAction::triggered, [this]() { showDialog(); });

    // There may be changes in the future.
    widgetManager->subscribe(
        "Quantify.start", msg.object,
        (SubscribeFunc)[this](const WEvent &) { showDialog(); });

    WMessage msg2;
    msg2.map[Key::MenuPath] = QVariant::fromValue(QString("Quantify"));
    msg2.object = action;
    WEvent extEvent(QString("ui.mainwindow.") + Event::MenuAction, msg2);
    widgetManager->publish(extEvent);
    return true;
}
///
/// \brief QuantifyPlugin::recMsg
/// \param msg
///
void QuantifyPlugin::recMsg(WMessage &msg) { Q_UNUSED(msg); }
///
/// \brief QuantifyPlugin::deinit
/// \param msg
/// \return
///
bool QuantifyPlugin::deinit(WMessage &msg) {
    Q_UNUSED(msg);
    return true;
}

void QuantifyPlugin::showDialog() {
    if (widget == nullptr) {
        widget = new QuantifyDialog;
        widget->setPlugin(this);
        widget->show();
    } else
        widget->activateWindow();
}

void QuantifyPlugin::restartDialog() {
    if (widget) {
        widget->close();
        widget = nullptr;
    }
    showDialog();
}