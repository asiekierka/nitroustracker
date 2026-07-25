/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

extern float sqrtf(float);

#include "platform.h"
#include "shader_shbin.h"
#include <3ds.h>
#include <citro3d.h>

#ifndef NT_3DS_FRAMEBUFFER

struct ctr_shader_data {
	DVLB_s *dvlb;
	shaderProgram_s program;
	int proj_loc;
	C3D_AttrInfo attr;
};

Screen *main_screen, *sub_screen;
static bool screensSwapped;
static bool mainScreenDrawRequested;

static C3D_RenderTarget *target_top, *target_bottom;
static C3D_Tex tex_main, tex_sub;
static C3D_Mtx proj_top, proj_bottom;
static struct ctr_shader_data shader;

void ctr_init_shader(struct ctr_shader_data *shader, const void *data, int size)
{
	shader->dvlb = DVLB_ParseFile((u32 *)data, size);
	shaderProgramInit(&shader->program);
	shaderProgramSetVsh(&shader->program, &shader->dvlb->DVLE[0]);
	shader->proj_loc = shaderInstanceGetUniformLocation(
	    shader->program.vertexShader, "projection");
	AttrInfo_Init(&shader->attr);
}

void ctr_bind_shader(struct ctr_shader_data *shader)
{
	C3D_BindProgram(&shader->program);
	C3D_SetAttrInfo(&shader->attr);
}

bool PlatformVideoInit(void)
{
	gfxInitDefault();
	gfxSet3D(false);
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

	target_top = C3D_RenderTargetCreate(240, 400, GPU_RB_RGB8, GPU_RB_DEPTH16);
	target_bottom =
	    C3D_RenderTargetCreate(240, 320, GPU_RB_RGB8, GPU_RB_DEPTH16);
	C3D_RenderTargetClear(target_top, C3D_CLEAR_ALL, 0, 0);
	C3D_RenderTargetClear(target_bottom, C3D_CLEAR_ALL, 0, 0);
	C3D_RenderTargetSetOutput(target_top, GFX_TOP, GFX_LEFT,
	                          GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGB8) |
	                              GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8));
	C3D_RenderTargetSetOutput(target_bottom, GFX_BOTTOM, GFX_LEFT,
	                          GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGB8) |
	                              GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8));

	C3D_TexInitVRAM(&tex_main, 512, 256, GPU_RGBA5551);
	C3D_TexInitVRAM(&tex_sub, 512, 256, GPU_RGBA5551);

	ctr_init_shader(&shader, shader_shbin, shader_shbin_size);
	AttrInfo_AddLoader(&(shader.attr), 0, GPU_FLOAT, 3); // v0 = position
	AttrInfo_AddLoader(&(shader.attr), 1, GPU_FLOAT, 2); // v1 = texcoord
	ctr_bind_shader(&shader);

	Mtx_OrthoTilt(&proj_top, 0.0, 400.0, 0.0, 240.0, -1.0, 1.0, true);
	Mtx_OrthoTilt(&proj_bottom, 0.0, 320.0, 0.0, 240.0, -1.0, 1.0, true);

	C3D_TexEnv *texEnv = C3D_GetTexEnv(0);
	C3D_TexEnvSrc(texEnv, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR,
	              GPU_PRIMARY_COLOR);
	C3D_TexEnvOpRgb(texEnv, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_COLOR,
	                GPU_TEVOP_RGB_SRC_COLOR);
	C3D_TexEnvOpAlpha(texEnv, GPU_TEVOP_A_SRC_ALPHA, GPU_TEVOP_A_SRC_ALPHA,
	                  GPU_TEVOP_A_SRC_ALPHA);
	C3D_TexEnvFunc(texEnv, C3D_Both, GPU_MODULATE);

	C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);

	u16 *fb_main = (u16 *)linearAlloc(512 * 256 * 2);
	u16 *fb_sub = (u16 *)linearAlloc(512 * 256 * 2);

	main_screen = new Screen(fb_main, 400, 240, 512);
	sub_screen = new Screen(fb_sub, 320, 240, 512);
	screensSwapped = false;

	return true;
}

void PlatformVideoExit(void)
{
	C3D_Fini();
	gfxExit();
}

static void PlatformUploadScreen(bool is_sub)
{
	Screen *screen = is_sub ? sub_screen : main_screen;
	C3D_Tex *tex = is_sub ? &tex_sub : &tex_main;

	GSPGPU_FlushDataCache(screen->pixels, 512 * 256 * 2);
	C3D_SyncDisplayTransfer(
	    (u32 *)screen->pixels, GX_BUFFER_DIM(512, 256), (u32 *)tex->data,
	    GX_BUFFER_DIM(tex->width, tex->height),
	    (GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) |
	     GX_TRANSFER_RAW_COPY(0) |
	     GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGB5A1) |
	     GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB5A1) |
	     GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO)));
}

static void PlatformDrawScreen(bool is_sub)
{
	Screen *screen = is_sub ? sub_screen : main_screen;
	C3D_Tex *tex = is_sub ? &tex_sub : &tex_main;

	int width, height;
	if (is_sub == screensSwapped) {
		if (screen->getWidth() < 400)
			C3D_RenderTargetClear(target_top, C3D_CLEAR_ALL, 0, 0);
		C3D_FrameDrawOn(target_top);
		C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, shader.proj_loc, &proj_top);
		width = 400;
		height = 240;
	} else {
		C3D_FrameDrawOn(target_bottom);
		C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, shader.proj_loc, &proj_bottom);
		width = 320;
		height = 240;
	}

	C3D_TexBind(0, tex);

	float xmin = (width - screen->getWidth()) / 2.0f;
	float ymin = (height - screen->getHeight()) / 2.0f;
	float xmax = xmin + screen->getWidth();
	float ymax = ymin + screen->getHeight();
	float txmax = ((float)screen->getWidth() / tex->width);
	float txmin = 0.0f;
	float tymin = ((float)screen->getHeight() / tex->height);
	float tymax = 0.0f;

	C3D_ImmDrawBegin(GPU_TRIANGLE_STRIP);
	C3D_ImmSendAttrib(xmin, ymin, 0.0f, 0.0f);
	C3D_ImmSendAttrib(txmin, tymin, 0.0f, 0.0f);

	C3D_ImmSendAttrib(xmax, ymin, 0.0f, 0.0f);
	C3D_ImmSendAttrib(txmax, tymin, 0.0f, 0.0f);

	C3D_ImmSendAttrib(xmin, ymax, 0.0f, 0.0f);
	C3D_ImmSendAttrib(txmin, tymax, 0.0f, 0.0f);

	C3D_ImmSendAttrib(xmax, ymax, 0.0f, 0.0f);
	C3D_ImmSendAttrib(txmax, tymax, 0.0f, 0.0f);
	C3D_ImmDrawEnd();
}

void PlatformFlipMainScreen(void)
{
	mainScreenDrawRequested = true;
}

void PlatformClearMainScreen(tobkit_pixel_t color)
{
	main_screen->clear(color);
	PlatformFlipMainScreen();
}

void PlatformClearSubScreen(tobkit_pixel_t color)
{
	sub_screen->clear(color);
}

void PlatformDrawSubScreen(void)
{
	if (gspHasGpuRight()) {
		PlatformUploadScreen(true);
		C3D_FrameBegin(0);
		PlatformDrawScreen(true);
		C3D_FrameEnd(0);
	}
}

bool PlatformWaitVBlank(void)
{
	if (!aptMainLoop())
		return false;
	if (gspHasGpuRight()) {
		if (mainScreenDrawRequested) {
			PlatformUploadScreen(false);
			mainScreenDrawRequested = false;
		}
		PlatformUploadScreen(true);
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		PlatformDrawScreen(false);
		PlatformDrawScreen(true);
		C3D_FrameEnd(0);
	}
	return true;
}

void PlatformVideoFadeIn(void)
{
}

bool PlatformVideoAreScreensSwapped(void)
{
	return screensSwapped;
}

bool PlatformVideoSwapScreens(void)
{
	screensSwapped = !screensSwapped;

	if (screensSwapped) {
		main_screen->setSize(320, 240, 512);
		sub_screen->setSize(320, 240, 512);
	} else {
		main_screen->setSize(400, 240, 512);
		sub_screen->setSize(320, 240, 512);
	}

	return true;
}

#endif
