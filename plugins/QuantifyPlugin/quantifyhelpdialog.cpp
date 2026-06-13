/**
 * @file quantifyhelptwindow.cpp
 * @brief 帮助对话框
 * @author howdy213
 * @date 2026-06-09
 * @version 2.1.1
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
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QGridLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMessageBox>
#include <QTextBrowser>

#include "WECore/metadata/WMetaDocument.h"
#include "WECore/def/wedef.h"

#include "quantifyhelpdialog.h"
#include "ui_quantifyhelpdialog.h"

using namespace we;
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
QuantifyHelpDialog::QuantifyHelpDialog(WMetaDocument *doc, QWidget *parent)
    : QDialog(parent), ui(new Ui::QuantifyHelpDialog), m_doc(doc) {
    ui->setupUi(this);
    setWindowTitle(tr("量化插件帮助"));
    resize(700, 500);

    setupIntroTab();
    setupQuickStartTab();
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

<h3>主要特性</h3>
<ul>
    <li><b>学生与分组管理</b>：通过学生名单 Excel 文件定义学生缩写与中文名，支持创建任意多个分组，便于批量操作。</li>
    <li><b>多周期自动汇总</b>：支持每日、每周、每学期三级周期，周期结束时自动调用规则计算累计分数。</li>
    <li><b>灵活的规则引擎</b>：
        <ul>
            <li><b>原生语法</b>：简洁的条件-动作表达式，适合常规量化场景。</li>
            <li><b>JavaScript 引擎</b>：提供完整的编程能力，可实现复杂逻辑（如分段扣分、复杂条件判断等）。</li>
        </ul>
    </li>
    <li><b>记录文件导入</b>：通过简单文本格式记录每次量化事件（迟到、表扬等），支持注释、组排除语法等。</li>
    <li><b>文件编辑器</b>：内置语法验证，错误提示，模板创建、快速录入等功能。</li>
    <li><b>直观的展示界面</b>：表格形式展示所有学生的每周评分和总分，支持排序、平均计算、导出 Excel、双击查看详情。</li>
    <li><b>配置文件驱动</b>：所有路径、引擎类型等均可通过 config.json 灵活配置。</li>
    <li><b>示例一键生成</b>：内置“新建示例”功能，快速生成完整目录结构和示例文件，方便上手。</li>
    <li><b>安全</b>：允许选择加密来阻止他人编辑，提供简短安全标识判断文件是否被篡改。</li>
</ul>
    )";
    browser->setHtml(wrapHtml(content));
}
///
/// \brief QuantifyHelpDialog::setupQuickStartTab
///
void QuantifyHelpDialog::setupQuickStartTab() {
    QTextBrowser *browser = findChild<QTextBrowser *>("textBrowserQuickStart");
    if (!browser)
        return;

    QString content = R"(
        <h2>快速入门</h2>
        <p>本指南将帮助你快速上手量化插件。请按照以下步骤操作，即可在几分钟内看到量化评分结果。</p>

        <h3>1. 创建示例数据</h3>
        <p>点击主界面的 <b>设置</b> 按钮，选择 <b>新建配置</b>。程序会要求你选择规则引擎类型：</p>
        <ul>
            <li><b>native</b>（原生语法）：适合简单规则，推荐初学者使用。</li>
            <li><b>js</b>（JavaScript）：适合复杂计算，需要一定编程基础。</li>
        </ul>
        <p>选择后程序会自动在插件目录下生成 <code>Quantify/config.json</code> 配置文件以及示例的 <code>rule</code>、<code>record</code> 等空文件夹。你可以直接使用示例数据继续体验。</p>
        <p>也可以导入更完整的示例数据以看到最终效果，已经包含在数据目录下，前往 <b>设置 - 还原数据</b> ，选择 <code>example</code> 或将 <b>操行路径</b> 改为 <code>./example</code> ，之后 <b>保存</b></p>
        <h3>2. 添加学生名单</h3>
        <p>在设置窗口中点击 <b>名单 - 打开</b>，程序会尝试打开数据目录下的 <code>namelist.xlsx</code> 文件。若文件不存在，可以用 Excel 新建一个，格式要求：</p>
        <ul>
            <li><b>第一列</b>：学生缩写（英文字母或数字，不能包含<code>-</code>符号），如 <code>zs</code></li>
            <li><b>第二列</b>：学生中文名，如 <code>张三</code></li>
        </ul>
        <p>保存并关闭 Excel 后，回到<b>查看 - 刷新</b>插件会自动读取名单。</p>

        <h3>3. 添加量化规则</h3>
        <p>切换到 <b>编辑</b> 标签页，在顶部的文件类型下拉框中选择 <b>rule</b>，然后在旁边的输入框中输入规则名（英文，例如 <code>late</code>）。</p>
        <p>如果你不熟悉规则语法，可以点击 <b>模板</b> 按钮快速填入示例。
        <p>编辑完成后，点击 <b>检查</b> 验证语法，确认无误后点击 <b>保存</b>。至此规则就配置好了。</p>

        <h3>4. 添加量化记录</h3>
        <p>同样在 <b>编辑</b> 标签页，将文件类型下拉框切换为 <b>record</b>。文件名遵循 <code>日期-序号</code> 的格式，例如 <code>20260531-1</code>（表示 2026 年 5 月 31 日的第一条记录）。</p>
        <p>记录文件内容由三部分组成：</p>
        <ul>
            <li><b>第一行</b>：记录类型（daily / weekly / termly）</li>
            <li><b>规则块</b>：以 <code>[规则英文名]</code> 开头，下方每行为 学生缩写 分数 备注（备注可选）</li>
            <li>支持行内注释 <code>//</code></li>
        </ul>
        <p>同样地，编辑后先 <b>检查</b> 再 <b>保存</b>。</p>
        <p><b>注意：</b>每过一周，你需要手动创建一个 <code>weekly</code> 类型的记录文件（文件名仍用日期，内容示例：<code>weekly\n[late]\nALL</code>），程序才能正确结算上周分数并开启新的一周，这允许不规则的周。可以在周六、周日创建。</p>

        <h3>5. （可选）添加小组</h3>
        <p>如果需要按小组批量操作，可以在编辑页面新建 <b>group</b> 类型文件。格式为：</p>
        <pre>组名 中文名
学生缩写1
学生缩写2
...</pre>
        <p>之后在记录文件中就可以使用组名作为目标。</p>

        <h3>6. 查看量化结果</h3>
        <p>完成以上步骤后，回到 <b>查看</b> 标签页，点击 <b>刷新</b> 按钮。表格会显示所有学生的每周得分和总分。双击某个分数单元格还可以查看该学生的详细记录。</p>

        <h3>常见问题与提示</h3>
        <ul>
            <li><b>记录文件加密？</b> 详见“安全设置”标签页。</li>
            <li><b>修改引擎类型？</b> 请注意不同引擎的规则文件格式完全不同，需要重新编写规则。</li>
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
            <li><b>查看详情</b>：双击某个学生的分数单元格，弹出该学生的详细记录窗口，可查看每次操作的日期、分数、原因、备注，并支持筛选和导出。<br>
            <li><b>汇总列</b>：会显示该周段内所有周的详细记录。</li>
        </ul>
        <p>表格包含“姓名”列、每周列、可自定义的汇总列、“总分”列。每周数据由规则文件自动计算得出。</p>
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
        <p><b>说明：</b></p>
        <ul>
            <li>日历中日期会显示三种颜色，代表当天有几个 <code>.record</code> 文件。</li>
            <li>双击日历日期可打开当天第一个记录文件；日历下方的 1、2、3 按钮可快速打开该日对应的三个记录文件（需先点击日历选中日期）。</li>
            <li>如果未保存被覆盖，可使用 <code>Ctrl+Z</code> 撤销。</li>
            <li><b>关于周数（week）的存储</b>：每条记录都会标记它属于哪一周（从 1 开始计数）。
                学期末（<code>termly</code> 类型）的记录会被归入最后一周的汇总中，方便您查看完整学期的总况。</li>
        </ul>
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

        <p><b>示例1：通用规则（每日加次数和分数）</b></p>
        <pre>function commonHandler(ctx) {
    ctx.record.t += 1;
    ctx.record.s += 0.5;
    return ctx;
}
({
    reason: "commonRule",
    reason_ch: "通用规则",
    daily: commonHandler,
    weekly: commonHandler,
    termly: commonHandler
})</pre>

        <p><b>示例2：多行为规则（每日累加，每周根据次数额外加分，学期末输出总评）</b></p>
        <pre>({
    reason: "multiBehavior",
    reason_ch: "多行为规则",
    daily: function(ctx) {
        ctx.record.t += 1;
        ctx.record.s += 0.2;
        return ctx;
    },
    weekly: function(ctx) {
        const lateCount = ctx.record.t2;
        if (lateCount >= 3) {
            ctx.record.s += 0.1;
        }
        return ctx;
    },
    termly: function(ctx) {
        const totalScore = ctx.record.s3;
        const formatted = totalScore.toFixed(2);
        if (totalScore >= 0.5) {
            ctx.log.message = `本规则学期总评：优秀(${formatted})`;
        } else if (totalScore >= 0.3) {
            ctx.log.message = `本规则学期总评：合格(${formatted})`;
        } else {
            ctx.log.message = `本规则学期总评：待改进(${formatted})`;
        }
        ctx.log.level = "info";
        return ctx;
    }
})</pre>

        <p><b>示例3：自定义规则（custom）</b><br>
        <code>custom</code> 规则是一个特殊规则，其记录文件中的分数会直接加减总分，不经过规则引擎计算。但仍需创建 <code>custom.rule</code> 文件，内容可为最简单的占位：</p>
        <pre>({
    reason: 'custom',
    reason_ch: '自定义',
    daily: function(record) { return record; },
    weekly: function(record) { return record; },
    termly: function(record) { return record; }
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
            <li>在“设置”窗口点击“生成密钥对”，选择 U 盘根目录，程序会在该 U 盘生成 <code>Quantify.pem</code> 私钥文件，并在插件目录生成对应的公钥 <code>public.pem</code>。</li>
            <li>将 U 盘插入运行插件的电脑，插件启动时自动读取私钥，即可对新建的记录文件加密。解密文件无需私钥。</li>
            <li>若 U 盘未插入或私钥无效，则无法修改文件。</li>
            <li><b>自定义公钥</b>：插件启动时会优先加载 <code>Quantify</code> 目录下的 <code>public.pem</code>（如果存在）；若不存在则使用内置默认公钥。因此您可以生成新的密钥对并用新公钥替换该文件，实现密钥更新。</li>
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
    "template": "模板文件路径（可选）"
}</pre>
        <ul>
            <li><b>path</b>：存放规则文件、记录文件、组文件以及 <code>namelist.xlsx</code> 的目录。可使用相对路径（相对于配置文件所在目录）或绝对路径。</li>
            <li><b>addon</b>：点击“工具”按钮时执行的程序路径（如打开外部编辑器或统计工具）。</li>
            <li><b>engine</b>：选择规则引擎，可选 <code>native</code>（原生语法）或 <code>js</code>（JavaScript）。修改后需重启插件生效。</li>
            <li><b>template</b>：模板文件目录，用于“模板”按钮加载预设内容。模板文件名分别为 <code>record.txt</code>（记录模板）、<code>rule-native.txt</code>（原生规则模板）和 <code>rule-js.txt</code>（JS规则模板）。</li>
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
        <p><b>版本：</b>2.1.1</p>
        <p><b>作者：</b>howdy213</p>
        <p><b>日期：</b>2026-05-31</p>
        <p><b>许可证：</b>GNU Lesser General Public License v3.0</p>
        <p>本插件用于学生日常行为量化评分，支持多种规则引擎，提供直观的显示和编辑界面。</p>
        <p>更多信息请参阅 <a href='https://github.com/howdy213/QuantifyPlugin'>GitHub 仓库</a>。</p>
    )";
    browser->setHtml(wrapHtml(content));
}
