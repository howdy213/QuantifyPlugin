/**
 * @file querymaindialog.cpp
 * @brief Implementation file for the main widget selection dialog.
 * @author howdy213
 * @date 2026-08-20
 * @version 2.0.0
 *
 * @copyright Copyright 2025-2026 howdy213
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
#include "querymaindialog.h"
#include "WECore/plugin/wplugin.h"
#include "WECore/we/we.h"
#include "WECore/file/wpath.h"
#include <WECore/metadata/WMetaDocument.h>
#include <WECore/we/webase.h>

using namespace we;
using namespace we::Consts;

QueryMainDialog::QueryMainDialog(QWidget *parent)
    : QDialog(parent), m_btnGroup(new QButtonGroup(this)), m_btnId(0) {
    setWindowTitle("选择MainWidget");
    setMinimumSize(300, 220);
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QLabel *tipLabel = new QLabel("请选择优先启动的MainWidget：", this);
    mainLayout->addWidget(tipLabel);

    m_contentLayout = new QVBoxLayout();
    mainLayout->addLayout(m_contentLayout);

    // 添加复选框
    m_defaultCheckBox = new QCheckBox("设为默认启动项", this);
    mainLayout->addWidget(m_defaultCheckBox);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *okBtn = new QPushButton("确定", this);
    QPushButton *cancelBtn = new QPushButton("取消", this);
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addSpacing(10);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    // 修改确定按钮的连接
    connect(okBtn, &QPushButton::clicked, this, &QueryMainDialog::onOkClicked);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    m_btnGroup->setExclusive(true);
}

void QueryMainDialog::addMainWidget(WPlugin* plugin) {
    if (!plugin)
        return;

    // 使用插件的 Path 元数据作为显示文本
    QString displayName = plugin->path();
    QRadioButton *radioBtn = new QRadioButton(displayName, this);
    m_contentLayout->addWidget(radioBtn);
    m_btnGroup->addButton(radioBtn, m_btnId);
    radioBtn->setProperty("pluginPtr", QVariant::fromValue<quintptr>(
                                           reinterpret_cast<quintptr>(plugin)));
    m_btnId++;

    adjustSize();
}

bool QueryMainDialog::hasSelectedPlugin() const {
    return m_btnGroup->checkedId() != -1;
}

WPlugin* QueryMainDialog::selectedPlugin() const {
    QAbstractButton *checkedBtn = m_btnGroup->checkedButton();
    if (!checkedBtn) {
        return nullptr;
    }
    quintptr ptr = checkedBtn->property("pluginPtr").value<quintptr>();
    return reinterpret_cast<WPlugin*>(ptr);
}

bool QueryMainDialog::isSetDefault() const {
    return m_defaultCheckBox->isChecked();
}

void QueryMainDialog::onOkClicked() {
    WPlugin *plugin = selectedPlugin();
    if (m_defaultCheckBox->isChecked() && plugin) {
        auto configManager = WE::inst()->getWEClass()->configManager();
        if (configManager) {
            configManager->set(Config::DefaultMain,
                               plugin->localUuid().toString());
            configManager->save(WPath().getModuleFolder() + Config::ConfigPath);
        }
    }
    accept();
}