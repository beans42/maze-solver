#include "includes.hpp"
#include "image_manip.hpp"
#include "dijkstra.hpp"

namespace g { //globals
	GLFWwindow* window = nullptr;
	
	uint8_t* file = nullptr;
	int file_size = 0;

	GLuint picture = 0;
	int pic_width = 0, pic_height = 0;

	bool pic_chosen = false;
	bool solved = false;
	
	image_manip* img = nullptr;
}

extern "C" void update_file(char* buffer, int length) {
	if (g::file)
		free(g::file);
	g::file = (uint8_t*)buffer;
	g::file_size = length;
	load_texture_from_file(g::file, g::file_size, &g::picture, &g::pic_width, &g::pic_height);
	if (!g::pic_chosen) {
		g::img = new image_manip(&g::picture, &g::pic_width, &g::pic_height);
		g::pic_chosen = true;
		g::solved = false;
	} else g::solved = false;
}

void loop() {
	static bool main_window_init = false;
	static bool path_value = false;
	static float path_cols[3] = { 0.f, 1.f, 0.f };
	static bool cost_map = false;
	static point_t start = { 0, 0 }, end = { 0, 0 };
	static bool show_whole_image = false;
	static int marker_size = 10;
	static int chosen_algo = 0;
	static solution_interface* algo = nullptr;
	static solution_interface* algos[] = { new dijkstra };

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	glfwSetWindowSize(g::window, canvas_get_width(), canvas_get_height());
	ImGui::SetNextWindowSize({ (float)canvas_get_width(), (float)canvas_get_height() });

	{
		ImGui::Begin("##main window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar);
		if (!main_window_init) {
			main_window_init = true;
			ImGui::SetWindowPos(ImVec2(0, 0));
		}
		
		if (ImGui::Button("browse..."))
			file_dialog();

		if (g::pic_chosen) {
			ImGui::Checkbox("show entire image", &show_whole_image);
			ImGui::SameLine();
			ImGui::Text("size: %dx%d", g::pic_width, g::pic_height);

			ImGui::SliderInt("marker size", &marker_size, 1, 100);
			ImGui::SliderInt("start x    ", &start.x, 0, g::pic_width - 1);
			ImGui::SliderInt("start y    ", &start.y, 0, g::pic_height - 1);
			ImGui::SliderInt("end x      ", &end.x, 0, g::pic_width - 1);
			ImGui::SliderInt("end y      ", &end.y, 0, g::pic_height - 1);

			ImGui::Combo("algorithm", &chosen_algo, "dijkstra's shortest path\0\0");
			algo = algos[chosen_algo];

			if (!cost_map) {
				ImGui::Checkbox("path color based on value", &path_value);

				if (!path_value) {
					ImGui::SameLine();
					ImGui::ColorEdit3("path color", &path_cols[0]);
				}
			}

			ImGui::Checkbox("draw cost map?", &cost_map);
			ImGui::SameLine();
			if (ImGui::Button("solve")) {
				load_texture_from_file(g::file, g::file_size, &g::picture, &g::pic_width, &g::pic_height);
				g::img->binarize_texture();
				if (cost_map) {
					const auto binary_maze = g::img->get_texture_as_bool_vector();
					const auto temp = algo->get_cost_map(binary_maze, { g::pic_width, g::pic_height }, start, end);
					g::img->darken_background();
					g::img->draw_points(temp);
				}
				else {
					const auto binary_maze = g::img->get_texture_as_bool_vector();
					unsigned max_path_value;
					const auto temp = algo->get_path(binary_maze, { g::pic_width, g::pic_height }, start, end, &max_path_value);
					std::vector<std::tuple<int, int, rgba_t>> points;
					for (auto& point : temp) {
						uint8_t r = uint8_t(std::get<2>(point) / (float)max_path_value * 255.f);
						points.push_back({ std::get<0>(point), std::get<1>(point), path_value ?
							rgba_t{ uint8_t(0xFF - r), r, 0x00, 0xFF } :
							rgba_t{ uint8_t(path_cols[0] * 255.f), uint8_t(path_cols[1] * 255.f), uint8_t(path_cols[2] * 255.f), 0xFF } });
					}
					g::img->darken_background();
					g::img->draw_points(points);
				}
				g::solved = true;
			}

			if (g::solved) {
				ImGui::SameLine();
				if (ImGui::Button("open in another window")) {
					const auto pixels = g::img->get_image_data();
					int png_mem_file_size;
					const auto png_in_mem = stbi_write_png_to_mem((uint8_t*)pixels.data(), g::pic_width * 4, g::pic_width, g::pic_height, 4, &png_mem_file_size);
					const auto data_uri = std::string("data:image/png;base64,") + base64_encode((const char*)png_in_mem, png_mem_file_size);
					free(png_in_mem);
					open_url(data_uri.c_str(), data_uri.length());
				}
			}

			ImGui::Separator();

			if (g::pic_chosen) {
				g::img->draw_markers(start, end, marker_size);
				if (show_whole_image) {
					const auto pos = ImGui::GetWindowPos();
					const auto size = ImGui::GetWindowSize();
					ImGui::GetCurrentContext()->CurrentWindow->DrawList->AddImage((void*)(intptr_t)g::picture, ImGui::GetCursorScreenPos(), ImVec2(pos.x + size.x - 5, pos.y + size.y - 5));
				} else ImGui::Image((void*)g::picture, ImVec2((float)g::pic_width, (float)g::pic_height));
			}
		}

		ImGui::End();
	}

	ImGui::Render();

	int display_w, display_h;
	glfwGetFramebufferSize(g::window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClear(GL_COLOR_BUFFER_BIT);

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int main() {
	glfwInit();

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	g::window = glfwCreateWindow(800, 600, "web-gui demo", NULL, NULL);
	glfwMakeContextCurrent(g::window);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	
	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.0f;
	style.FrameRounding = 5.f;
	style.GrabRounding = style.FrameRounding;

	ImGui_ImplGlfw_InitForOpenGL(g::window, false);
	ImGui_ImplOpenGL3_Init();

	ImGui::StyleColorsDark();

	glClearColor(0.45f, 0.55f, 0.60f, 1.00f);

	glfwSetMouseButtonCallback(g::window, ImGui_ImplGlfw_MouseButtonCallback);
	glfwSetScrollCallback(g::window, ImGui_ImplGlfw_ScrollCallback);
	glfwSetKeyCallback(g::window, ImGui_ImplGlfw_KeyCallback);
	glfwSetCharCallback(g::window, ImGui_ImplGlfw_CharCallback);

	resize_canvas();

	emscripten_set_main_loop(loop, 0, 1);

	glfwTerminate();
}