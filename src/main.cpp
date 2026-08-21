/**
 * @file main.cpp
 * @brief 程序入口文件，实现了程序的初始化与主流程控制
 * @author howdy213
 * @date 2026-08-20
 * @version 2.0.0
 *
 * Copyright 2025-2026 howdy213
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
 */
#include "PluginConfigWidget.h"
#include "lightwidget.h"

#include "WECore/file/wpath.h"
#include "WECore/metadata/WMetaDocument.h"
#include "WECore/we/we.h"

#include <QApplication>
#include <QDialog>
#include <QLabel>
#include <QLockFile>
#include <QMenu>
#include <QMessageBox>
#include <QStyleFactory>
#include <QSystemTrayIcon>
#include <QVBoxLayout>
#include <WECore/file/wshellexecute.h>

using namespace we::Consts;
using namespace we;

// 函数声明
QStringList processParams(int argc, char *argv[], bool &pluginManagerMode);
bool handleQtEnvironment(WMetaDocument *config);
int handlePluginConfigManager(PluginConfigManager *configManager);
int initMainPlugin(LightWidget *base, QStringList params,
                   WMetaDocument *config);

/**
 * @brief 应用程序主入口函数
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return 程序退出码，0 表示正常退出，非 0 表示异常或用户主动退出
 *
 * 负责完成以下工作：
 * 1. 检查单例锁，防止重复运行
 * 2. 解析命令行参数，判断是否为插件配置管理模式
 * 3. 初始化核心系统（WE 框架、LightWidget）
 * 4. 加载全局配置并设置 Qt 环境变量
 * 5. 根据运行模式执行相应流程
 */
int main(int argc, char *argv[]) {
    // 单例锁：避免程序重复启动
    QLockFile lockfile(WPath().getModuleFolder() + "we.lock");
    if (!lockfile.tryLock(0))
        return 1;

    bool pluginManagerMode = false;
    QStringList params = processParams(argc, argv, pluginManagerMode);

    // 创建 LightWidget 实例并初始化核心系统
    auto lw = new LightWidget;
    if (!WE::init(lw))
        return 0;
    if (!lw->init())
        return 0;

    // 加载全局配置文件
    lw->getWEClass()->configManager()->load(
        WPath().getModuleFolder() + Config::ConfigPath, true);
    auto config = lw->getWEClass()->configManager();

    // 根据配置设置 Qt 环境变量（必须在 QApplication 创建前完成）
    handleQtEnvironment(config);

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/icons/icon/we.png"));

    // 插件配置管理模式
    if (pluginManagerMode)
        return handlePluginConfigManager(lw->pluginConfigManager());

    // 正常启动模式
    if (!initMainPlugin(lw, params, config))
        return 1;  // 无可用的主插件，退出

    return a.exec();
}

/**
 * @brief 解析命令行参数，提取运行模式与剩余参数
 * @param argc 参数个数
 * @param argv 参数数组
 * @param pluginManagerMode 输出参数，指示是否启用插件配置管理模式
 * @return 去除模式标志后剩余的参数列表（通常是主插件需要的参数）
 *
 * 支持识别 `-pluginmanager` 或 `--pluginmanager` 参数。
 * 其余参数将被收集并返回，供后续主插件使用。
 */
QStringList processParams(int argc, char *argv[], bool &pluginManagerMode) {
    QStringList params;
    for (int i = 1; i < argc; ++i) {
        QString arg = QString::fromLocal8Bit(argv[i]);
        if (arg == "-pluginmanager" || arg == "--pluginmanager") {
            pluginManagerMode = true;
            continue;
        }
        params.append(arg);
    }
    return params;
}

/**
 * @brief 根据配置设置 Qt 环境变量（如字体 DPI、缩放因子）
 * @param config 配置对象
 * @return 始终返回 true
 *
 */
bool handleQtEnvironment(WMetaDocument *config) {
    if (/*qEnvironmentVariableIsEmpty("QT_FONT_DPI")*/ true) {
        if (config->hasArg(Config::Font))
            qputenv("QT_FONT_DPI",
                    qvariant_cast<QByteArray>(config->get(Config::Font)));
        if (config->hasArg(Config::Scale))
            qputenv("QT_SCALE_FACTOR",
                    qvariant_cast<QByteArray>(config->get(Config::Scale)));
    }
    return true;
}

/**
 * @brief 处理插件配置管理器模式
 * @param configManager 插件配置管理器实例指针
 * @return 程序退出码
 *
 * 该模式用于单独编辑插件配置文件，不会加载任何插件。
 * 显示编辑器对话框，编辑完成退出当前进程。
 */
int handlePluginConfigManager(PluginConfigManager *configManager) {
    QDialog dlg;
    dlg.setWindowTitle("插件配置管理器");
    dlg.resize(800, 600);

    // 创建配置编辑控件并设置根配置文件路径
    PluginConfigWidget *configWidget =
        new PluginConfigWidget(configManager, &dlg);
    QString rootConfigPath = WPath().getModuleFolder() + Plugins::ConfigPath;
    configWidget->setRootJsonPath(rootConfigPath);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    layout->addWidget(configWidget);

    QLabel *tipLabel = new QLabel("安全模式：当前未加载任何插件。", &dlg);
    layout->addWidget(tipLabel);

    dlg.exec();

    // 编辑完成后询问是否重启主程序
    QMessageBox::StandardButton ret = QMessageBox::question(
        nullptr, QStringLiteral("插件配置管理器"),
        QStringLiteral("配置编辑完成，是否重新启动主程序？"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (ret != QMessageBox::Yes) {
        return 0;
    }

    WShellExecute::asyncExecute(
        WPath().getModuleFolder() + "tools/WELauncher.exe", "open", "-t 500");
    return 0;
}

/**
 * @brief 初始化并启动主插件
 * @param base LightWidget 实例指针
 * @param params 从命令行解析出的参数列表，将传递给主插件
 * @param config 全局配置对象
 * @return 成功返回 1，失败返回 0
 *
 * 从配置中读取默认主插件的 LocalUuid（若无则为空），
 * 调用 LightWidget::initMainPlugin 尝试加载并初始化主插件。
 * 若初始化失败，弹出提示对话框并返回 0。
 */
int initMainPlugin(LightWidget *base, QStringList params,
                   WMetaDocument *config) {
    QUuid defaultMain = QUuid();
    if (config->hasArg(Config::DefaultMain))
        defaultMain = QUuid(base->getWEClass()
                                ->configManager()
                                ->get(Config::DefaultMain)
                                .toString());

    if (!base->initMainPlugin(params, defaultMain)) {
        QMessageBox::StandardButton ret = QMessageBox::question(
            nullptr, QStringLiteral("提示"),
            QStringLiteral("无可用主界面，是否打开插件配置管理器？"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes);
        if (ret == QMessageBox::Yes) {
            return handlePluginConfigManager(base->pluginConfigManager());
        } else {
            return 0;
        }
    }
    return 1;
}