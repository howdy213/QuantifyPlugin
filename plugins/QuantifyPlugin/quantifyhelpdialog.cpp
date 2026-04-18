/**
 * @file quantifyhelptwindow.cpp
 * @brief 帮助对话框
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
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QGridLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMessageBox>
#include <QTextBrowser>

#include "WConfig/wconfigdocument.h"
#include "WECore/WDef/wedef.h"

#include "quantifyhelpdialog.h"
#include "ui_quantifyhelpdialog.h"

using namespace we::Consts;

namespace {
///
/// @brief 样式表
////
const QString STYLE_SHEET = R"(
        <style>
            body {
                font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif;
                font-size: 14px;
                line-height: 1.6;
                color: #24292e;
                background-color: #ffffff;
                margin: 20px;
            }
            h2 {
                font-size: 1.6em;
                border-bottom: 2px solid #eaecef;
                padding-bottom: 0.3em;
                color: #0366d6;
                margin-top: 24px;
                margin-bottom: 16px;
                font-weight: 600;
            }
            h3 {
                font-size: 1.3em;
                margin-top: 24px;
                margin-bottom: 16px;
                font-weight: 600;
                color: #24292e;
            }
            p {
                margin-top: 0;
                margin-bottom: 16px;
            }
            ul, ol {
                padding-left: 2em;
                margin-bottom: 16px;
            }
            li {
                margin: 0.25em 0;
            }
            pre {
                background-color: #f6f8fa;
                border-radius: 6px;
                padding: 16px;
                overflow: auto;
                font-family: "SF Mono", Monaco, Consolas, "Liberation Mono", Courier, monospace;
                font-size: 85%;
                line-height: 1.45;
                border: 1px solid #e1e4e8;
                margin-bottom: 16px;
                box-shadow: 0 1px 2px rgba(0,0,0,0.04);
            }
            code {
                background-color: rgba(27,31,35,0.05);
                border-radius: 3px;
                font-family: "SF Mono", Monaco, Consolas, "Liberation Mono", Courier, monospace;
                padding: 0.2em 0.4em;
                font-size: 85%;
                color: #d73a49;
                border: 1px solid #e1e4e8;
            }
            pre code {
                background-color: transparent;
                padding: 0;
                color: inherit;
                border: none;
            }
            b {
                color: #0366d6;
            }
            table {
                border-collapse: collapse;
                width: 100%;
                margin: 16px 0;
                font-size: 90%;
            }
            th, td {
                border: 1px solid #dfe2e5;
                padding: 6px 13px;
                text-align: left;
            }
            th {
                background-color: #f6f8fa;
                font-weight: 600;
            }
            tr:nth-child(even) {
                background-color: #f6f8fa;
            }
        </style>
    )";

///
/// @brief 将正文内容包装为完整的 HTML 文档，应用统一的样式表
/// @param bodyContent 要嵌入 <body> 中的 HTML 片段
/// @return 完整的 HTML 字符串
///
QString wrapHtml(const QString &bodyContent) {
    return QStringLiteral("<html><head>%1</head><body>%2</body></html>")
        .arg(STYLE_SHEET, bodyContent);
}
} // namespace

///
/// @class QuantifyHelpDialog
/// @brief
/// 量化插件帮助对话框，包含多个标签页，展示插件功能、文件格式、配置说明等
///
QuantifyHelpDialog::QuantifyHelpDialog(WConfigDocument *doc, QWidget *parent)
    : QDialog(parent), ui(new Ui::QuantifyHelpDialog), m_doc(doc) {
    ui->setupUi(this);
    setWindowTitle(tr("量化插件帮助"));
    resize(700, 500);

    setupIntroTab();
    setupQuantifyTab();
    setupRecordTab();
    setupRuleTab();
    setupGroupTab();
    setupConfigTab();
    setupNamelistTab();
    setupSecurityTab();
    setupAboutTab();
}
///
/// \brief QuantifyHelpDialog::~QuantifyHelpDialog
///
QuantifyHelpDialog::~QuantifyHelpDialog() { delete ui; }
///
/// \brief QuantifyHelpDialog::setupIntroTab
///
void QuantifyHelpDialog::setupIntroTab() {
    QTextBrowser *browser = findChild<QTextBrowser *>("textBrowserIntro");
    if (!browser)
        return;

    QString content = R"(
        <h2>量化插件功能介绍</h2>
        <p>量化插件用于记录和管理学生日常行为量化评分，支持多种规则引擎（原生语法和 JavaScript），并提供直观的显示和编辑界面。</p>
        <h3>主要特性：</h3>
        <ul>
            <li>支持学生和组的管理</li>
            <li>每日/每周/每学期自动汇总评分</li>
            <li>可自定义量化规则（原生语法或 JS）</li>
            <li>记录查看、筛选、导出 Excel</li>
            <li>配置文件灵活设置路径和引擎类型</li>
        </ul>
    )";
    browser->setHtml(wrapHtml(content));
}
///
/// \brief QuantifyHelpDialog::setupQuantifyTab
///
void QuantifyHelpDialog::setupQuantifyTab() {
    QTextBrowser *browser = findChild<QTextBrowser *>("textBrowserQuantify");
    if (!browser)
        return;

    QString content = R"(
        <h2>展示页面</h2>
        <p>展示页面以表格形式显示所有学生的每周评分和总分，支持以下操作：</p>
        <ul>
            <li><b>刷新</b>：重新读取所有规则和记录文件，更新数据。</li>
            <li><b>导出</b>：将当前表格导出为 Excel 文件（.xlsx）。</li>
            <li><b>排序</b>：点击列标题可按该列排序（姓名按字母，分数按数值降序）。</li>
            <li><b>查看详情</b>：双击某个学生的分数单元格，弹出该学生的详细记录窗口，可查看每次操作的日期、分数、原因、备注，并支持筛选和导出。</li>
        </ul>
        <p>表格包含“姓名”列、每周列以及“总分”列。每周数据由规则文件自动计算得出。</p>
    )";
    browser->setHtml(wrapHtml(content));
}
///
/// \brief QuantifyHelpDialog::setupGroupTab
///
void QuantifyHelpDialog::setupGroupTab() {
    QTextBrowser *browser = findChild<QTextBrowser *>("textBrowserGroup");
    if (!browser)
        return;

    QString content = R"(
        <h2>组文件（.group）格式</h2>
        <p>组文件用于定义学生分组，方便批量操作。文件格式如下：</p>
        <pre>组名 中文名
学生缩写1
学生缩写2
...</pre>
        <p>示例：</p>
        <pre>group1 第一组
zs
ls
ww</pre>
        <p>组名在记录文件中可作为目标使用，如</p>
        <pre>[custom]
group1 +0.5 小组加分</pre>
        <p>会使该组所有成员获得加分。特别地，预定义组 <code>ALL</code> 代表全体学生，无需额外定义即可使用。</p>
    )";
    browser->setHtml(wrapHtml(content));
}
///
/// \brief QuantifyHelpDialog::setupRecordTab
///
void QuantifyHelpDialog::setupRecordTab() {
    QTextBrowser *browser = findChild<QTextBrowser *>("textBrowserRecord");
    if (!browser)
        return;

    QString content = R"(
        <h2>记录文件（.record）格式</h2>
        <p>记录文件用于导入某次具体的量化操作，每行代表一条记录。文件名即为日期名，如20260303-1.record。<br>文件格式如下：</p>
        <pre>类型（daily / weekly / termly）
[规则名] [额外信息]（可选）
目标 分数 备注（可选）
...</pre>
        <p><b>类型</b>：指定本次记录所属周期，影响规则中对应周期的函数调用。</p>
        <p><b>规则名</b>：必须与某个规则文件的 <code>reason</code> 匹配，用方括号括起，如 <code>[late]</code>。其后可添加额外信息，整个块内的行都会附加该信息。</p>
        <p><b>目标</b>：学生缩写、组名(不能包含'-'符号)，或组排除语法。<br>组排除语法格式为 <code>组名-排除成员1-排除成员2...</code>，表示该组中除指定成员外的所有学生。</p>
        <p><b>分数</b>：格式为 <code>+0.5</code> 或 <code>-1</code>，表示加减分数。对于 <code>custom</code> 规则，此处分数值会直接传递给规则处理。</p>
        <p><b>备注</b>：可选，记录附加说明。</p>
        <p>支持行内注释：以 <code>//</code> 开头的内容将被忽略。</p>
        <p>示例：</p>
        <pre>daily  // 这是每日记录
[late]  // 迟到规则
zs 早自习迟到//备注
ls 迟到
[custom][ 额外信息]
zs -0.2  // 自定义扣分
group1-s1-s2 -0.1 // 代表group1中除了s1、s2都-0.1
[clean][ 值日]
ls
ww</pre>
        <p>上述示例等价于：</p>
        <pre>daily
[late]
zs 早自习迟到
ls 迟到
[custom]
zs -0.2 额外信息
group1-s1-s2 -0.1 额外信息
[clean]
ls 值日
ww 值日</pre>
        <p>注意：<code>[custom]</code> 是一个特殊规则，其分数部分直接作用于学生总分，不经过规则引擎计算，但仍然需要创建 custom.rule 文件并按规范格式填入（虽然会忽略其中定义的任何东西）。</p>
        <p>出现 daily 类型会清空 t1, s1 变量；出现 weekly 类型会清空 t1/t2, s1/s2 变量，并在展示页增加一列。不建议在weekly/termly中进行个人/小组加减分，而是作为每周/学期结算日，如：</p>
        <pre>weekly
[late]//调用late的weekly规则
ALL
[assembly]
ALL
[homework]
ALL</pre>
    )";
    browser->setHtml(wrapHtml(content));
}
///
/// \brief QuantifyHelpDialog::setupRuleTab
///
void QuantifyHelpDialog::setupRuleTab() {
    QTextBrowser *introBrowser =
        findChild<QTextBrowser *>("textBrowserRuleIntro");
    QTextBrowser *nativeBrowser =
        findChild<QTextBrowser *>("textBrowserRuleNative");
    QTextBrowser *jsBrowser = findChild<QTextBrowser *>("textBrowserRuleJs");

    if (!introBrowser || !nativeBrowser || !jsBrowser)
        return;

    QString introContent = R"(
        <h2>规则文件概述</h2>
        <p>规则文件定义了每个量化项目的计算逻辑，有两种语法引擎可选：<b>原生语法</b>和<b>JavaScript</b>，JavaScript引擎可以提供更复杂的计算（例如分段扣分，若不会编写可将规则介绍提供给AI并提出要求）。<br>在配置文件 <code>config.json</code> 中设置 <code>"engine"</code> 为 <code>"native"</code> 或 <code>"js"</code> 来切换。</p>
        <p>文件名即为规则英文名（但不强制，程序不参考文件名）。如 <code>late.rule</code> 对应规则 <code>late</code>。规则文件需放置在 <code>path</code> 目录下的 <code>rule</code> 子目录中。</p>

        <h3>核心变量说明</h3>
        <p>每个学生的记录包含以下六个基本变量：</p>
        <ul>
            <li><b>t1</b>：每日次数累计</li>
            <li><b>t2</b>：每周次数累计</li>
            <li><b>t3</b>：每学期次数累计</li>
            <li><b>s1</b>：每日分数累计</li>
            <li><b>s2</b>：每周分数累计</li>
            <li><b>s3</b>：每学期分数累计</li>
        </ul>
        <p>此外，为了方便操作，引入了简写：</p>
        <ul>
            <li><b>t</b>：同时操作 t1、t2、t3</li>
            <li><b>s</b>：同时操作 s1、s2、s3</li>
            <li>在原生语法中会实时修改三个变量；在 JS 中会在返回后计算，表示本次调用的变化量</li>
        </ul>
        <p><b>注意：</b>每个周期结束时，系统会调用对应周期的规则，传入当前学生的记录对象，规则可以读取和修改这些变量。修改后的值会持久化存储，并在后续周期中继续累计。</p>
        <p>例如，若每日迟到一次，希望每日分数减0.5，同时每周累计迟到次数超过3次后额外扣分，则需要在每日规则和每周规则中分别处理。</p>
        <p><b>注意：</b>在函数中直接修改 <code>t1,t2,t3,s1,s2,s3</code> 变量也是允许的，但需注意，不完整的处理会导致周分数相加不等于总分数。因此，无特殊要求，建议变量<code>t1,t2,t3,s1,s2,s3</code>只读，<code>t,s</code>只写</p>
     )";
    introBrowser->setHtml(wrapHtml(introContent));

    QString nativeContent = R"(
        <h2>原生语法</h2>
        <p>文件结构如下（每组规则以单独的 <code>-</code> 行分隔，若某规则有多行，则按顺序执行，即使某组为空也须保留分隔符）：</p>
        <pre>英文名 中文名
-
每日规则行1
每日规则行2
-
每周规则行
-
每学期规则行
-</pre>

        <p><b>规则行语法：</b> <code>[条件]:[动作1]:[动作2]...</code></p>
        <ul>
            <li><b>条件</b>：使用 <code>&gt;</code>、<code>&lt;</code>、<code>=</code> 比较变量和常量，可用 <code>&amp;</code>（与）、<code>|</code>（或）组合多个条件。例如 <code>t1&gt;2&amp;s2&lt;5</code> 表示“每日次数大于2且每周分数小于5”。</li>
            <li><b>动作</b>：使用 <code>+</code>、<code>-</code>、<code>=</code> 对变量进行增减或赋值，多个动作以冒号分隔。例如 <code>t1+1:s2-0.5</code> 表示“每日次数加1，每周分数减0.5”。</li>
            <li><b>简写</b>：<code>t</code> 和 <code>s</code> 表示同时操作对应的三个具体变量，例如 <code>t+1</code> 等价于 <code>t1+1:t2+1:t3+1</code>。</li>
        </ul>

        <p>示例1：每次迟到扣0.5分</p>
        <pre>late 迟到
-
:s-0.5
-
-
-</pre>

        <p>示例2：本周迟到三次及以上才开始扣分</p>
        <pre>late 迟到
-
t2&gt;2:s-0.5
-
-
-</pre>

        <p>示例3：同时操作次数和分数，并带条件</p>
        <pre>class-good 课堂表现好
-
t1&gt;0&amp;s2&lt;2:s+0.2
-
-
-</pre>
    )";
    nativeBrowser->setHtml(wrapHtml(nativeContent));

    QString jsContent = R"(
        <h2>JavaScript 引擎</h2>
        <p>规则文件需导出一个包含 <code>reason</code>、<code>reason_ch</code> 以及 <code>daily</code>、<code>weekly</code>、<code>termly</code> 函数的对象。
        <br>每个函数接收一个 <code>ctx</code> 对象，该对象包含 <code>record</code> 和 <code>log</code> 两个属性。</p>

        <p><b>关于简写 <code>t</code> 和 <code>s</code>：</b></p>
        <ul>
            <li>它们的初始值总是 <b>0</b>，表示本次调用中累计的变化量。</li>
            <li>您可以对它们进行赋值或加减操作，例如 <code>ctx.record.t += 1</code>，这些操作只会在返回后展开到 <code>t1,t2,t3</code> 上（即等价于分别加1）。</li>
            <li>函数必须返回 <code>ctx</code> 对象（或其修改后的副本）。</li>
        </ul>

        <p>示例（集会加分规则）：</p>
        <pre>({
    reason: 'assembly+',
    reason_ch: '集会',
    daily: function(ctx) {
        ctx.record.t += 1;
        ctx.record.s += 0.1;
        if(ctx.record.t1 > 1.5){
            ctx.log.level = "warning";
            ctx.log.message = "一天不能集会两次";
        }
        return ctx;
    },
    weekly: function(ctx) {
        // 无规则
        return ctx;
    },
    termly: function(ctx) {
        // 无规则
        return ctx;
    },
})</pre>
    )";
    jsBrowser->setHtml(wrapHtml(jsContent));
}
///
/// \brief QuantifyHelpDialog::setupGroupTab
///
void QuantifyHelpDialog::setupSecurityTab() {
    QTextBrowser *browser = findChild<QTextBrowser *>("textBrowserSecurity");
    if (!browser)
        return;

    QString content = R"(
        <h2>安全设置</h2>
        <p>为防止记录文件被直接篡改，插件支持对 <code>.record</code> 文件进行加密。</p>
        <ul>
            <li><b>无</b>：不进行加密，记录文件为明文。</li>
            <li><b>加密</b>：使用 AES-256-GCM 算法加密记录文件，密钥存储在外部移动设备（如 USB 密钥）中，每次启动插件时读取。</li>
        </ul>
        <p>加密流程：</p>
        <ul>
            <li>在“设置”窗口点击“生成密钥对”，选择 U 盘根目录，程序会在该 U 盘生成 <code>Quantify.pem</code> 私钥文件，并在插件目录生成对应的公钥。</li>
            <li>将 U 盘插入运行插件的电脑，插件启动时自动读取私钥，即可对新建的记录文件加密。解密文件无需私钥。</li>
            <li>若 U 盘未插入或私钥无效，则无法修改文件。</li>
            <li>当前公钥文件内置，暂时无法更改，可以在软件包中获取对应的示例密钥。</li>
        </ul>
        <p>加密记录文件以 <code>QCRY</code> 魔数开头，无法直接查看内容。</p>

        <h3>编辑窗口安全标识</h3>
        <p>在编辑窗口底部会显示类似 <code>A1 2026-4-5 12:30:00:111 20260405-1.record 3/5</code> 的信息：</p>
        <ul>
            <li><b>前两位十六进制数</b>（如 <code>A1</code>）：表示所有记录文件列表的哈希值（SHA256首字节），辅助发现文件是否被删除或添加。</li>
            <li><b>时间戳及文件名</b>（如 <code>2026-4-5 12:30:00:111 20260405-1.record</code>）：表示所有记录文件中最后修改时间最晚的日期时间及对应文件名，辅助发现文件是否被外部修改。</li>
            <li><b>加密计数</b>（如 <code>3/5</code>）：表示 5 个记录文件中有 3 个已加密。</li>
        </ul>
        <p>该标识可用于快速判断记录文件是否被外部篡改，或是否与当前显示的数据同步。</p>
    )";
    browser->setHtml(wrapHtml(content));
}
///
/// \brief QuantifyHelpDialog::setupConfigTab
///
void QuantifyHelpDialog::setupConfigTab() {
    QTextBrowser *browser = findChild<QTextBrowser *>("textBrowserConfig");
    if (!browser)
        return;

    QString content = R"(
        <h2>配置文件说明</h2>
        <p>配置文件位于插件目录下的 <code>Quantify/config.json</code>，内容如下：</p>
        <pre>{
    "path": "数据目录路径",
    "addon": "附加程序路径（可选）",
    "engine": "规则引擎类型：native 或 js",
    "template": "模板文件路径（可选）",
    "encryption": true/false
}</pre>
        <ul>
            <li><b>path</b>：存放规则文件、记录文件、组文件以及 <code>namelist.xlsx</code> 的目录。可使用相对路径（相对于配置文件所在目录）或绝对路径。</li>
            <li><b>addon</b>：点击“工具”按钮时执行的程序路径（如打开外部编辑器或统计工具）。</li>
            <li><b>engine</b>：选择规则引擎，可选 <code>native</code>（原生语法）或 <code>js</code>（JavaScript）。修改后需重启插件生效。</li>
            <li><b>template</b>：模板文件目录，用于“模板”按钮加载预设内容。模板文件名分别为 <code>record.txt</code>（记录模板）、<code>rule-native.txt</code>（原生规则模板）和 <code>rule-js.txt</code>（JS规则模板）。</li>
            <li><b>encryption</b>：是否启用记录文件加密（<code>true</code> 启用，<code>false</code> 禁用）。启用后，新保存的 <code>.record</code> 文件将使用 AES-256-GCM 加密，需要插入包含私钥的 U 盘才能正常读写。</li>
        </ul>
        <p><b>路径注意事项：</b>路径中的反斜杠请使用 <code>/</code> 或双反斜杠 <code>\\</code>。相对路径基于 <code>config.json</code> 所在目录。</p>
        <p><b>加密相关：</b>若启用加密，请先在“设置”页面生成密钥对至 U 盘。插件启动时会自动检测 U 盘根目录下的 <code>Quantify.pem</code> 私钥文件。未插入 U 盘或私钥无效时，无法加密新记录，但可以解密已有加密文件。</p>
    )";
    browser->setHtml(wrapHtml(content));
}
///
/// \brief QuantifyHelpDialog::setupNamelistTab
///
void QuantifyHelpDialog::setupNamelistTab() {
    QTextBrowser *browser = findChild<QTextBrowser *>("textBrowserNamelist");
    if (!browser)
        return;

    QString content = R"(
        <h2>学生名单文件（namelist.xlsx）</h2>
        <p>学生名单须以 Excel 文件形式存放在 <code>config.json</code>中<code>path</code> 目录下，文件名为 <code>namelist.xlsx</code>。格式如下：</p>
        <ul>
            <li>第一列：学生缩写（英文/字母，用于记录文件中的目标）</li>
            <li>第二列：学生中文名（显示在表格中）</li>
        </ul>
        <p>示例：</p>
        <table>
            <tr><td>zs</td><td>张三</td></tr>
            <tr><td>ls</td><td>李四</td></tr>
            <tr><td>ww</td><td>王五</td></tr>
        </table>
    )";
    browser->setHtml(wrapHtml(content));
}
///
/// \brief QuantifyHelpDialog::setupAboutTab
///
void QuantifyHelpDialog::setupAboutTab() {
    QTextBrowser *browser = findChild<QTextBrowser *>("textBrowserAbout");
    if (!browser)
        return;
    browser->setOpenExternalLinks(true);
    QString content = R"(
        <h2>量化插件 QuantifyPlugin</h2>
        <p><b>版本：</b>1.5.0</p>
        <p><b>作者：</b>howdy213</p>
        <p><b>日期：</b>2026-04-18</p>
        <p><b>许可证：</b>GNU General Public License v3.0</p>
        <p>本插件用于学生日常行为量化评分，支持多种规则引擎，提供直观的显示和编辑界面。</p>
        <p>更多信息请参阅 <a href='https://github.com/howdy213/QuantifyPlugin'>GitHub 仓库</a>。</p>
    )";
    browser->setHtml(wrapHtml(content));
}
