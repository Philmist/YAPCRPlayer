/*
 * (C) 2015 see Authors.txt
 *
 * This file is part of MPC-BE.
 *
 * MPC-BE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-BE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Some shaders are taken from Jan-Willem Krans "Video pixel shader pack v1.4" and "MPC-HC tester builds".

#pragma once

// Nearest neighbor
extern char const shader_resizer_nearest[];

// Bilinear
extern char const shader_resizer_bilinear[];

// Perlin Smootherstep
extern char const shader_resizer_smootherstep[];

// Bicubic
extern char const shader_resizer_bicubic[];

// B-spline4
extern char const shader_resizer_bspline4[];

// Mitchell-Netravali spline4
extern char const shader_resizer_mitchell4[];

// Catmull-Rom spline4
extern char const shader_resizer_catmull4[];

// downscaling
extern char const shader_resizer_downscaling[];

// Final pass
extern char const shader_final[];


// Two-pass resizers

// Bicubic
extern char const shader_resizer_bicubic_2pass[];

// B-spline4
extern char const shader_resizer_bspline4_2pass[];

// Mitchell-Netravali spline4
extern char const shader_resizer_mitchell4_2pass[];

// Catmull-Rom spline4
extern char const shader_resizer_catmull4_2pass[];

// compensated Lanczos2
extern char const shader_resizer_lanczos2_2pass[];

// downscaling
extern char const shader_resizer_downscaling_2pass[];

// compensated Lanczos3
extern char const shader_resizer_lanczos3_2pass[];
