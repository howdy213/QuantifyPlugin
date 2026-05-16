/**
 * @file virtualkeyboard.h
 * @brief Virtual keyboard control header.
 * @author howdy213
 * @date 2026-05-04
 * @version 2.0.0
 *
 * @copyright Copyright (C) 2025-2026 howdy213
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
#ifndef VIRTUALKEYBOARD_H
#define VIRTUALKEYBOARD_H

/**
 * @class VirtualKeyboard
 * @brief Provides methods to open the system virtual keyboard.
 */
class VirtualKeyboard
{
public:
    VirtualKeyboard() = default;
    ~VirtualKeyboard() = default;

    bool OpenScreenKeyboard();
    bool OpenOSK();
    bool OpenTabTip();
    bool IsNewVersion();
};

#endif // VIRTUALKEYBOARD_H