# ClassPoints

## 简介
ClassPoints 是一个用于学生日常行为量化评分的插件。它支持灵活的规则配置，可记录每次加/减分操作，并按日、周、学期自动汇总统计，最终以表格形式展示每位学生的各项评分及总分。插件提供原生语法和 JavaScript 两种规则引擎，满足从简单到复杂的评分逻辑需求。使用 WECore 插件系统以支持功能扩展。

## 主要特性
- **学生与分组管理**：通过学生名单 Excel 文件定义学生缩写与中文名，支持创建任意多个分组，便于批量操作。
- **多周期自动汇总**：支持每日、每周、每学期三级周期以及自定义时间段，周期结束时自动调用规则计算累计分数。
- **灵活的规则引擎**：
  - **原生语法**：简洁的条件-动作表达式，适合常规量化场景。
  - **JavaScript 引擎**：提供完整的编程能力，可实现复杂逻辑（如分段扣分、复杂条件判断等）。
- **记录文件导入**：通过简单文本格式记录每次量化事件（迟到、表扬等），支持注释、组排除语法等。
- **文件编辑器**：内置语法验证，错误提示，模板创建、快速录入等功能。
- **直观的展示界面**：表格形式展示所有学生/小组的每周评分和总分，支持排序、平均计算、导出 Excel、双击查看详情、搜索、按规则类型折叠、按成员折叠（小组分数中）等。
- **配置持久化保存**：所有配置均可通过设置界面和 `config.json` 灵活管理。
- **示例一键生成**：内置“新建示例”功能，快速生成完整目录结构和示例文件，方便上手。
- **安全**：允许选择加密来阻止他人编辑，提供简短安全标识判断文件是否被篡改。
- **备份还原**: 支持完整备份所有数据以便迁移。

## 安装与配置

### 文件结构
插件安装后，其目录结构通常如下：
```
QuantifyPlugin.dll
Quantify/
├── config.json                  主配置文件
├── addon/                       附加工具程序存放目录
├── template/                    模板文件目录
│   ├── record.txt               记录文件模板
│   ├── rule-native.txt          原生规则模板
│   ├── rule-js.txt              JS规则模板
│   └── default.txt              编辑界面的默认展示内容
└── [学期目录]/                   例如 term1（由用户指定，可有多个）
    ├── namelist.xlsx            学生名单
    ├── rule/                    规则文件存放处
    │   ├── late.rule
    │   ├── custom.rule
    │   └── ...
    ├── record/                   记录文件存放处
    │   ├── 20250301-1.record
    │   └── ...
    └── group/                    组文件存放处
        ├── group1.group
        └── ...
```

### 配置文件 config.json
位于插件根目录下，JSON 格式：
```json
{
    "path": "./term1",                 // 规则/记录/组文件所在目录（相对 config.json 或绝对路径）
    "addon": "./addon",                // 附加程序路径（点击“工具”按钮时打开）
    "engine": "native",                // 规则引擎类型：native 或 js
    "template": "./template",          // 模板文件目录
}
```
**注意**：文件中路径中的反斜杠请使用 `/` 或双反斜杠 `\\` （转义），在设置界面编辑则只需单斜杠。

## 帮助
软件功能及使用方式在内置帮助页有详细说明，内含示例数据以便快速了解。

下载 `ClassPoints.zip` ，双击 `WidgetExplorer.exe`  即可使用。

## 依赖

- Qt6
- [WECore](https://github.com/howdy213/WECore)：WidgetExplorer插件核心库
- [WidgetExplorer](https://github.com/howdy213/WidgetExplorer)
- [QXlsx](https://github.com/QtExcel/QXlsx)：Qt Xlsx格式读写库
- [OpenSSL](https://github.com/openssl/openssl): 加密库

## 许可证
本项目采用 LGPLv3许可证，详情参见 [LICENSE](LICENSE) 文件。

WECore: [Apache-2.0 license](licenses/LICENSE-WECore)

WidgetExplorer: [Apache-2.0 license](licenses/LICENSE-WidgetExplorer)

QXlsx: [MIT License](licenses/LICENSE-QXlsx)

OpenSSL: [Apache-2.0 license](licenses/LICENSE-OpenSSL)

Qt: [LGPLv3 许可证](licenses/LICENSE.LESSER-Qt)