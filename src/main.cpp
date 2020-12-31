#include "includes.hpp"
#include "image_manip.hpp"

#include "algos/dijkstra.hpp"
#include "algos/a_star.hpp"
#include "algos/breadth_first.hpp"
#include "algos/depth_first.hpp"

#ifdef _WIN32
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "windows-deps/GLEW/lib/glew32s.lib")
#pragma comment(lib, "windows-deps/GLFW/lib/glfw3.lib")
#pragma comment(linker, "/subsystem:windows")
int WinMain() {
#else
int main() {
#endif
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(800, 600, "maze solver native", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	glewInit();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().IniFilename = nullptr;

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.0f;
	style.FrameRounding = 5.f;
	style.GrabRounding = style.FrameRounding;

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	bool main_window_init = false;
	bool path_value = false;
	float path_cols[3] = { 0.f, 1.f, 0.f };
	bool cost_map = false;
	point_t start = { 0, 0 }, end = { 0, 0 };
	bool show_whole_image = false;
	int marker_size = 10;

	std::string file_name = "";
	GLuint picture = 0;
	unsigned pic_width = 0, pic_height = 0;
	bool pic_chosen = false;
	bool solved = false;
	image_manip* img = nullptr;

	int chosen_algo = 0;
	solution_interface* algo = nullptr;
	solution_interface* algos[] = { new dijkstra, new a_star, new breadth_first, new depth_first };

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		int window_width, window_height;
		glfwGetWindowSize(window, &window_width, &window_height);
		ImGui::SetNextWindowSize({ (float)window_width, (float)window_height });

		{
			ImGui::Begin("##main window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar);
			if (!main_window_init) {
				main_window_init = true;
				ImGui::SetWindowPos(ImVec2(0, 0));
			}

			if (ImGui::Button("upload maze...")) {
				file_name = get_file_name();
				if (file_name == "") {}
				else if (!pic_chosen) {
					load_texture_from_file(file_name.c_str(), &picture, &pic_width, &pic_height);

					img = new image_manip(&picture, &pic_width, &pic_height);
					pic_chosen = true;
					solved = false;
				}
				else {
					load_texture_from_file(file_name.c_str(), &picture, &pic_width, &pic_height);
					solved = false;
				}
			}

			if (pic_chosen) {
				ImGui::Checkbox("show entire image", &show_whole_image);
				ImGui::SameLine();
				ImGui::Text("size: %dx%d", pic_width, pic_height);

				ImGui::SliderInt("marker size", &marker_size, 1, 100);
				ImGui::SliderInt("start x    ", &start.x, 0, pic_width - 1);
				ImGui::SliderInt("start y    ", &start.y, 0, pic_height - 1);
				ImGui::SliderInt("end x      ", &end.x, 0, pic_width - 1);
				ImGui::SliderInt("end y      ", &end.y, 0, pic_height - 1);

				ImGui::RadioButton("dijkstra", &chosen_algo, 0); ImGui::SameLine();
				ImGui::RadioButton("a* search", &chosen_algo, 1); ImGui::SameLine();
				ImGui::RadioButton("breadth first", &chosen_algo, 2); ImGui::SameLine();
				ImGui::RadioButton("depth first", &chosen_algo, 3);
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
					load_texture_from_file(file_name.c_str(), &picture, &pic_width, &pic_height);
					img->binarize_texture();
					const auto binary_maze = img->get_texture_as_bool_vector();
					auto ret = algo->solve({ pic_width, pic_height, start, end, binary_maze });
					if (ret.solved) {
						std::vector<std::tuple<int, int, rgba_t>> points;
						if (cost_map) {
							std::replace(ret.cost_map.begin(), ret.cost_map.end(), UINT_MAX, 0u);
							const auto max_distance = *std::max_element(ret.cost_map.begin(), ret.cost_map.end());
							const auto multiplier = (double)1 / (double)max_distance;
							for (int i = 0; i < ret.cost_map.size(); ++i) {
								const auto color = uint8_t((double)255 * (double)multiplier * (double)ret.cost_map[i]);
								points.push_back({ int(i % pic_width), int(i / pic_width), {color, color, color, 0xFF} });
							}
						}
						else {
							for (int i = 0; i < ret.path.size(); ++i) {
								const auto r = uint8_t((float)i / (float)ret.path.size() * 255.f);
								points.push_back({ ret.path[i].x, ret.path[i].y, path_value ?
									rgba_t{ r, 0, uint8_t(0xFF - r), (uint8_t)0xFF } :
									rgba_t{ uint8_t(path_cols[0] * 255.f), uint8_t(path_cols[1] * 255.f), uint8_t(path_cols[2] * 255.f), 0xFF } });
							}
						}
						img->darken_background();
						img->draw_points(points);
						solved = true;
					}
					else tinyfd_messageBox("alert", "no solution found", "info", "info", 1);
				}

				if (solved) {
					ImGui::SameLine();
					if (ImGui::Button("save as jpg")) {
						size_t last_period = file_name.find_last_of(".");
						std::string raw_name = file_name.substr(0, last_period);
						raw_name += "_output.jpg";
						const auto vec_image = img->get_image_data();
						stbi_write_jpg(raw_name.c_str(), pic_width, pic_height, 4, vec_image.data(), 100);
					}
				}

				ImGui::Separator();

				ImGui::BeginChild("##maze display");
				if (pic_chosen) {
					img->draw_markers(start, end, marker_size);
					if (show_whole_image) {
						const auto pos = ImGui::GetWindowPos();
						const auto size = ImGui::GetWindowSize();
						ImGui::GetCurrentContext()->CurrentWindow->DrawList->AddImage((void*)(intptr_t)picture, ImGui::GetCursorScreenPos(), ImVec2(pos.x + size.x - 5, pos.y + size.y - 5));
					}
					else ImGui::Image((void*)picture, ImVec2((float)pic_width, (float)pic_height));
				}
				ImGui::EndChild();
			}

			ImGui::End();
		}

		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
