/**
 * @file virtualkeyboard.h
 * @brief 虚拟键盘
 * @author howdy213
 * @date 2026-3-1
 * @version 1.3.0
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
#ifndef VIRTUALKEYBOARD_H
#define VIRTUALKEYBOARD_H

class VirtualKeyboard
{
public:
    VirtualKeyboard() = default;
    ~VirtualKeyboard() = default;

    bool OpenScreenKeyboard();
    bool OpenOSK();
    bool OpenTabTip();
    bool IsWin10KeyboardVisable();
    bool IsWin7KeyboardVisable();
    // 判断系统版本是否大于或等于win10 10.0.14393.0
    bool IsNewVersion();
};

#endif // VIRTUALKEYBOARD_H
