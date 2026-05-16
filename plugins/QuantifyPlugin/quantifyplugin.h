/**
 * @file quantifyplugin.h
 * @brief 插件类
 * @author howdy213
 * @date 2026-05-16
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
#ifndef QUANTIFYPLUGIN_H
#define QUANTIFYPLUGIN_H
#include "WECore/plugin/wpluginmessage.h"
#include "WECore/plugin/wplugininterface.h"
#include "quantifydialog.h"

#include <QObject>
#include <QtPlugin>
class QuantifyPlugin : public QObject, public WPluginInterface
{
public:
    Q_OBJECT
    Q_PLUGIN_METADATA(IID WPluginInterface_iid FILE "QuantifyPlugin.json")
    Q_INTERFACES(WPluginInterface)

public:
    QuantifyPlugin();
    ~QuantifyPlugin();
    bool init(we::WMessage &msg) override;
    void recMsg(we::WMessage &msg) override;
    bool deinit(we::WMessage& msg) override;
    void setWidget(QuantifyDialog *widget) { this->widget = widget; };

private:
    void showDialog();
    QuantifyDialog* widget=nullptr;
    we::WEBase *we = nullptr;
};

#endif // QUANTIFYPLUGIN_H
