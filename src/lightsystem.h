/**
 * @file lightsystem.h
 * @brief 轻量插件加载系统头文件
 * @author howdy213
 * @date 2026-1-30
 * @version 1.1.0
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
#ifndef LIGHTSYSTEM_H
#define LIGHTSYSTEM_H
#include "WECore/WDef/wedef.h"

#include <QString>

class LightSystemPrivate;

class WE_NAMESPACE::LightSystem {
public:
    LightSystem();
    virtual ~LightSystem();
    void loadAllPlugin();
    void loadPlugin(QString jsonPath, QString config);
    void loadSinglePlugin(QString jsonPath, WConfigDocument *doc);

private:
    LightSystemPrivate *d = nullptr;
};

#endif // LIGHTSYSTEM_H
