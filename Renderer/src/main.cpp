#include "pch.h"
// 窗口
#include "Platform/Windows/Win32.h"
// 相机
#include "Renderer/Core/Camera/Camera.h"
// 着色器
#include "Renderer/Core/Shader/Shader.h"
// 采样函数
#include "Renderer/Core/Sample/Sample.h"
// 管线
#include "Renderer/Core/Pipeline/Pipeline.h"
// 场景
#include "Assets/scenes/Scene.h"

using namespace Renderer;

const vec3 Eye(0, 1, 5);
const vec3 Up(0, 1, 0);
const vec3 Target(0, 1, 0);

const scene_t Scenes[]
{
	{"fuhua",build_fuhua_scene},
	{"qiyana",build_qiyana_scene},
	{"yayi",build_yayi_scene},
	{"xier",build_xier_scene},
	{"helmet",build_helmet_scene},
	{"gun",build_gun_scene},
};

void clear_zbuffer(int width, int height, float* zbuffer);
void clear_framebuffer(int width, int height, unsigned char* framebuffer);
void update_matrix(Camera& camera, mat4 view_mat, mat4 perspective_mat, IShader* shader_model, IShader* shader_skybox);

int main()
{
	// 初始化

	// 申请空间(color buffer & z buffer)
	int width = WINDOW_WIDTH, height = WINDOW_HEIGHT;
	float* zbuffer = (float*)malloc(sizeof(float) * width * height);
	unsigned char* framebuffer = (unsigned char*)malloc(sizeof(unsigned char) * width * height * 4);
	memset(framebuffer, 0, sizeof(unsigned char) * width * height * 4);

	// 创建相机
	Camera camera(Eye, Target, Up, (float)(width) / height);

	// 设置mvp矩阵
	mat4 model_mat = mat4::identity();
	mat4 view_mat = mat4_lookat(camera.eye, camera.target, camera.up);
	mat4 perspective_mat = mat4_perspective(60, (float)(width) / height, -0.1, -10000);

	// 初始化模型和shader，创建scene
	srand((unsigned int)time(NULL));
	int scene_index = rand() % 6;
	int model_num = 0;
	Model* model[MAX_MODEL_NUM];
	IShader* shader_model;
	IShader* shader_skybox;
	Scenes[scene_index].build_scene(model, model_num, &shader_model, &shader_skybox, perspective_mat, &camera);

	// 初始化窗口
	window_init(width, height, "Renderer");

	// 渲染循环
	int num_frames = 0;
	float print_time = platform_get_time();
	while (!window->is_close)
	{
		float curr_time = platform_get_time();

		// 清空color buffer & z buffer
		clear_framebuffer(width, height, framebuffer);
		clear_zbuffer(width, height, zbuffer);

		// 处理事件（更新vp矩阵）
		handle_events(camera);
		update_matrix(camera, view_mat, perspective_mat, shader_model, shader_skybox);

		// 绘制模型
		for (int m = 0; m < model_num; m++)
		{
			// 将模型数据输入到shader中
			shader_model->payload.model = model[m];
			if (shader_skybox != NULL) shader_skybox->payload.model = model[m];

			// 根据模型类型选择不同的shader
			IShader* shader;
			if (model[m]->is_skybox)
				shader = shader_skybox;
			else
				shader = shader_model;

			// 绘制三角形
			for (int i = 0; i < model[m]->nfaces(); i++)
			{
				draw_triangles(framebuffer, zbuffer, *shader, i);
			}
		}

		// FPS
		num_frames += 1;
		if (curr_time - print_time >= 1) {
			int sum_millis = (int)((curr_time - print_time) * 1000);
			int avg_millis = sum_millis / num_frames;
			printf("fps: %3d, avg: %3d ms\n", num_frames, avg_millis);
			num_frames = 0;
			print_time = curr_time;
		}

		// 鼠标
		window->mouse_info.wheel_delta = 0;
		window->mouse_info.orbit_delta = vec2(0, 0);
		window->mouse_info.fv_delta = vec2(0, 0);

		// 将color buffer中的值传送到窗口中
		window_draw(framebuffer);
		msg_dispatch();
	}


	// 释放内存
	for (int i = 0; i < model_num; i++)
		if (model[i] != NULL)  delete model[i];
	if (shader_model != NULL)  delete shader_model;
	if (shader_skybox != NULL) delete shader_skybox;
	free(zbuffer);
	free(framebuffer);
	window_destroy();

	system("pause");
	return 0;
}


void clear_zbuffer(int width, int height, float* zbuffer)
{
	for (int i = 0; i < width * height; i++)
		zbuffer[i] = 100000;
}

void clear_framebuffer(int width, int height, unsigned char* framebuffer)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int index = (i * width + j) * 4;

			framebuffer[index + 2] = 80;
			framebuffer[index + 1] = 56;
			framebuffer[index] = 56;
		}
	}
}

void update_matrix(Camera& camera, mat4 view_mat, mat4 perspective_mat, IShader* shader_model, IShader* shader_skybox)
{
	view_mat = mat4_lookat(camera.eye, camera.target, camera.up);
	mat4 mvp = perspective_mat * view_mat;
	shader_model->payload.camera_view_matrix = view_mat;
	shader_model->payload.mvp_matrix = mvp;

	if (shader_skybox != NULL)
	{
		mat4 view_skybox = view_mat;
		view_skybox[0][3] = 0;
		view_skybox[1][3] = 0;
		view_skybox[2][3] = 0;
		shader_skybox->payload.camera_view_matrix = view_skybox;
		shader_skybox->payload.mvp_matrix = perspective_mat * view_skybox;
	}
}