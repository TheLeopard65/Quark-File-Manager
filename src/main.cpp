#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define GLFW_INCLUDE_NONE
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <sstream>
#include <string>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <iomanip>
#include <chrono>

#ifdef linux
const char* SYSTEM = "UNIX";
const char* HOMEDIR = "HOME";
const char* SLASH = "/";
const char* OPEN_APP = "xdg-open ";
#endif

#ifdef _WIN32
#include <Windows.h>
#include <cstdlib>
const char* SYSTEM = "WINDOWS";
const char* HOMEDIR = "HOMEPATH";
const char* SLASH = "\\";
const char* OPEN_APP = "start ";
void HideConsole(){ ::ShowWindow(::GetConsoleWindow(), SW_HIDE); }
#endif

GLuint folder_icon = 0;
GLuint file_icon = 0;
GLuint go_back_icon = 0;

struct OPTIONS {
    std::string path = getenv(HOMEDIR);
    std::string clicked_path = "";
    std::string copy_path    = "";
    int width  = 1600;
    int height = 900;
    char search_path[100]      = "";
    char new_folder_name[100]  = "";
    char new_file_name[100]    = "";
    bool is_app_options        = false;
    bool is_menu_options       = false;
    bool is_dir_options        = false;
    bool show_hidden           = false;
    bool is_cut                = false;
    bool is_rename             = false;
    bool is_create_folder      = false;
    bool is_create_file        = false;
    ImVec2 mouse_pos;
};
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void main_window(OPTIONS& options, ImGuiWindowFlags file_manager_window_flags, ImGuiWindowFlags options_window_flags);
void left_window(std::string& path, std::string* bookmarks, ImGuiWindowFlags bookmarks_window_flags);
GLuint LoadTextureFromFile(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);
    if (!data) {
        std::cerr << "Failed to load image: " << filename << std::endl;
        return 0;
    }
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    return tex;
}

int main() {
    #ifdef _WIN32
        HideConsole();
    #endif
    OPTIONS options;
    std::string bookmarks[8] = {
        options.path,
        options.path + SLASH + "Desktop",
        options.path + SLASH + "Documents",
        options.path + SLASH + "Downloads",
        options.path + SLASH + "Music",
        options.path + SLASH + "Pictures",
        options.path + SLASH + "Videos",
        options.path + SLASH + "Public"
    };

    assert(glfwInit() && "Could not init GLFW!");
    GLFWwindow *window = glfwCreateWindow(options.width, options.height, "QUARK FILE MANAGER", nullptr, nullptr);
    glfwSetErrorCallback([](int error, const char* description) { fprintf(stderr, "Error: %s\n", description); });
    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags = ImGuiViewportFlags_IsPlatformMonitor;
	ImFont* bannerFont = io.Fonts->AddFontFromFileTTF("../resources/Courier-New.ttf", 120.0f);
	ImFont* customFont = io.Fonts->AddFontFromFileTTF("../resources/Courier-New.ttf", 40.0f);
	if (!customFont || !bannerFont) std::cerr << "Failed to load fonts.\n";
	else io.FontDefault = customFont;
	folder_icon = LoadTextureFromFile("../resources/folder.png");
	file_icon   = LoadTextureFromFile("../resources/file.png");
	go_back_icon = LoadTextureFromFile("../resources/go_back.png");
    ImGui::StyleColorsDark();
    ImGui::GetStyle().FramePadding.y = 10.0f;
    ImGui::GetStyle().ItemSpacing.y = 10.0f;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 120");
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool show_demo_window = false;
    ImGuiWindowFlags main_window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar;
    ImGuiWindowFlags bookmarks_window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;
    ImGuiWindowFlags file_manager_window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;
    ImGuiWindowFlags options_window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0,0));
        ImGui::SetNextWindowSize(ImVec2{(float) options.width, (float) options.height});
        ImGui::Begin("MainWindow", 0, main_window_flags);
		{
		    if (bannerFont) ImGui::PushFont(bannerFont);
		    const char* bannerText = "QUARK FILE MANAGER";
		    ImVec2 windowSize = ImGui::GetWindowSize();
		    ImVec2 textSize = ImGui::CalcTextSize(bannerText);
		    ImGui::SetCursorPosX((windowSize.x - textSize.x) * 0.5f);
		    ImGui::Text("%s", bannerText);
		    if (bannerFont) ImGui::PopFont();
		    ImGui::Separator();
		    left_window(options.path, bookmarks, bookmarks_window_flags);
		    main_window(options, file_manager_window_flags, options_window_flags);
		}
        glfwGetWindowSize(window, &options.width, &options.height);
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    return 0;
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
}
void left_window(std::string& path, std::string* bookmarks, ImGuiWindowFlags bookmarks_window_flags) {
    std::string intermediate = "";
    ImGui::SameLine();
    ImGui::SetCursorPosX(0);
    if (ImGui::BeginChild("Bookmarks", ImVec2(350, 0), true, bookmarks_window_flags)) {
        for (int i = 0; i < 8; i++) {
            if (bookmarks[i] != getenv(HOMEDIR)) {
                intermediate = bookmarks[i].substr(bookmarks[i].rfind(SLASH)).erase(0, 1);
                ImGui::BeginGroup();
				ImGui::Image((void*)(intptr_t)folder_icon, ImVec2(38, 38));
				ImGui::SameLine(10, 35);
				ImGui::Selectable(intermediate.c_str(), false, 0, ImVec2(0, 40.0f));
				ImGui::EndGroup();
			} else {
			    ImGui::BeginGroup();
			    ImGui::Image((void*)(intptr_t)folder_icon, ImVec2(38, 38));
			    ImGui::SameLine(20, 25);
			    ImGui::Selectable("HOME", false, 0, ImVec2(0, 40.0f));
			    ImGui::EndGroup();
			}
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0) && std::filesystem::is_directory(bookmarks[i])) path = bookmarks[i];
        }
        ImGui::EndChild();
    }
}
void main_window(OPTIONS& options, ImGuiWindowFlags file_manager_window_flags, ImGuiWindowFlags options_window_flags) {
    std::string intermediate = "";
    bool is_hidden = false;
    ImGui::SameLine();
    ImGui::SetCursorPosX(350);
    if (ImGui::BeginChild("File Manager", ImVec2(0, 0), true, file_manager_window_flags)) {
    	ImGui::BeginGroup();
        ImGui::BeginDisabled(options.path == getenv(HOMEDIR));
        if (ImGui::ImageButton((void*)(intptr_t)go_back_icon, ImVec2(50, 40))) options.path = options.path.substr(0, options.path.rfind(SLASH));
        ImGui::EndDisabled();
        ImGui::SameLine(0, 30);
        if (ImGui::Button("OPTIONS", ImVec2(0, 0))) {
            options.mouse_pos.x = ImGui::GetWindowPos().x + 100;
            options.mouse_pos.y = ImGui::GetWindowPos().y + 50;
            options.is_menu_options = !options.is_menu_options;
        }
        ImGui::SameLine(0, 30);
        ImGui::PushItemWidth(options.width - 400);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
        ImGui::InputText("##pwd", (char*)options.path.c_str(), options.path.size() + 1, ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll);
        ImGui::PopStyleColor();
        ImGui::PopItemWidth();
        ImGui::EndGroup();
        if (ImGui::BeginTable("file_table", 5, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchSame)) {
		    ImGui::TableSetupColumn("NAME");
		    ImGui::TableSetupColumn("SIZE", ImGuiTableColumnFlags_WidthFixed, 150.0f);
		    ImGui::TableSetupColumn("LAST MODIFIED", ImGuiTableColumnFlags_WidthFixed, 450.0f);
		    ImGui::TableSetupColumn("PERMISSIONS", ImGuiTableColumnFlags_WidthFixed, 250.0f);
		    ImGui::TableSetupColumn("TYPE", ImGuiTableColumnFlags_WidthFixed, 100.0f);
		    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
			const char* headers[] = { "NAME", "SIZE", "LAST MODIFIED", "PERMISSIONS", "TYPE" };
			for (int column = 0; column < 5; column++) {
				ImGui::TableSetColumnIndex(column);
				const char* header = headers[column];
				float columnWidth = ImGui::GetColumnWidth();
				ImVec2 textSize = ImGui::CalcTextSize(header);
				float cursorX = ImGui::GetCursorPosX();
				ImGui::SetCursorPosX(cursorX + (columnWidth - textSize.x) * 0.5f);
				ImGui::TextUnformatted(header);
			}
		    for (auto& i : std::filesystem::directory_iterator(options.path)) {
		        std::string intermediate;
		        bool is_hidden = false;
		        std::stringstream line(i.path().string());
		        while (std::getline(line, intermediate, '/'))
		            if (intermediate.substr(0, 1) == ".") is_hidden = true;
		        if (is_hidden && !options.show_hidden) continue;
		        std::string name = i.path().filename().string();
		        bool is_dir = std::filesystem::is_directory(i.path());
		        auto ftime = std::filesystem::last_write_time(i.path());
		        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
		        std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
		        std::stringstream time_ss;
		        time_ss << std::put_time(std::localtime(&cftime), "%Y-%m-%d %H:%M:%S");
		        std::string size_str = "-";
		        if (!is_dir) size_str = std::to_string(std::filesystem::file_size(i.path())) + "B";
		        std::filesystem::perms p = std::filesystem::status(i.path()).permissions();
		        std::string permissions;
		        permissions += ((p & std::filesystem::perms::owner_read)  != std::filesystem::perms::none) ? "r" : "-";
		        permissions += ((p & std::filesystem::perms::owner_write) != std::filesystem::perms::none) ? "w" : "-";
		        permissions += ((p & std::filesystem::perms::owner_exec) != std::filesystem::perms::none) ? "x" : "-";
		        permissions += ((p & std::filesystem::perms::group_read)  != std::filesystem::perms::none) ? "r" : "-";
		        permissions += ((p & std::filesystem::perms::group_write) != std::filesystem::perms::none) ? "w" : "-";
		        permissions += ((p & std::filesystem::perms::group_exec) != std::filesystem::perms::none) ? "x" : "-";
		        permissions += ((p & std::filesystem::perms::others_read)  != std::filesystem::perms::none) ? "r" : "-";
		        permissions += ((p & std::filesystem::perms::others_write) != std::filesystem::perms::none) ? "w" : "-";
		        permissions += ((p & std::filesystem::perms::others_exec) != std::filesystem::perms::none) ? "x" : "-";
		        ImGui::TableNextRow();
		        ImGui::TableSetColumnIndex(0);
		        ImGui::PushID(i.path().string().c_str());
		        ImGui::BeginGroup();
				float iconPaddingLeft = 10.0f;
				float iconPaddingRight = 10.0f;
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + iconPaddingLeft);
				ImGui::Image((void*)(intptr_t)(is_dir ? folder_icon : file_icon), ImVec2(40, 40));
				ImGui::SameLine(0, iconPaddingRight);
				bool clicked = ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, 40.0f));
				ImGui::EndGroup();
				if (clicked) {
				    if (ImGui::IsMouseDoubleClicked(0)) {
				        if (is_dir) options.path = i.path().string();
				        else std::system((OPEN_APP + i.path().string()).c_str());
				    }
				    options.clicked_path = i.path().string();
				}
				if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
				    options.clicked_path = i.path().string();
				    options.is_app_options = true;
				    options.is_dir_options = false;
				    options.mouse_pos = ImGui::GetMousePos();
				}
				ImGui::TableSetColumnIndex(1);
				{
				    float columnWidth = ImGui::GetColumnWidth();
				    ImVec2 textSize = ImGui::CalcTextSize(size_str.c_str());
				    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - textSize.x) * 0.5f);
				    ImGui::Text("%s", size_str.c_str());
				}
				ImGui::TableSetColumnIndex(2);
				{
				    std::string timeStr = time_ss.str();
				    float columnWidth = ImGui::GetColumnWidth();
				    ImVec2 textSize = ImGui::CalcTextSize(timeStr.c_str());
				    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - textSize.x) * 0.5f);
				    ImGui::Text("%s", timeStr.c_str());
				}
				ImGui::TableSetColumnIndex(3);
				{
				    float columnWidth = ImGui::GetColumnWidth();
				    ImVec2 textSize = ImGui::CalcTextSize(permissions.c_str());
				    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - textSize.x) * 0.5f);
				    ImGui::Text("%s", permissions.c_str());
				}
				ImGui::TableSetColumnIndex(4);
				{
				    const char* type = is_dir ? "DIR" : "FILE";
				    float columnWidth = ImGui::GetColumnWidth();
				    ImVec2 textSize = ImGui::CalcTextSize(type);
				    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - textSize.x) * 0.5f);
				    ImGui::Text("%s", type);
				}
		        ImGui::PopID();
		    }
		    ImGui::EndTable();
		}
        if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(1)) {
            options.is_dir_options = true;
            options.is_app_options = false;
            options.mouse_pos = ImGui::GetMousePos();
            options.clicked_path = options.path;
        }
        if (options.is_app_options) {
            if (options.width - options.mouse_pos.x < 200) options.mouse_pos.x -= 200;
            if (options.height - options.mouse_pos.y < 200) options.mouse_pos.y -= 200;
            if (ImGui::IsMouseClicked(0) && (ImGui::GetMousePos().x < options.mouse_pos.x || ImGui::GetMousePos().x > options.mouse_pos.x + 200 ||
                ImGui::GetMousePos().y < options.mouse_pos.y || ImGui::GetMousePos().y > options.mouse_pos.y + 200)) {
                options.is_app_options = false;
            }
            ImGui::SetNextWindowPos(options.mouse_pos);
            ImGui::SetNextWindowSize(ImVec2(275, 510));
            ImGui::Begin("**options", &options.is_app_options, options_window_flags);
            ImGui::Selectable("Open", false, 0, ImVec2(0, 40));
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
                if (std::filesystem::is_directory(options.clicked_path)) options.path = options.clicked_path;
                else std::system((OPEN_APP + options.clicked_path).c_str());
                options.is_app_options = false;
            }
            ImGui::Selectable("New folder", false, 0, ImVec2(0, 40));
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) options.is_create_folder = true;
            if (options.is_create_folder) {
			    ImGui::SetKeyboardFocusHere();
			    if (ImGui::InputText("##new_folder_name", options.new_folder_name, 100, 
			        ImGuiInputTextFlags_EnterReturnsTrue)) {
			        auto parent = std::filesystem::path(options.clicked_path).parent_path();
			        auto new_dir = parent / options.new_folder_name;
			        std::filesystem::create_directory(new_dir);
			        options.is_app_options = options.is_create_folder = false;
			        options.new_folder_name[0] = '\0';
			    }
			    ImGui::SameLine();
			    if (ImGui::Button("Ok") && strlen(options.new_folder_name) > 0) {
			        auto parent = std::filesystem::path(options.clicked_path).parent_path();
			        auto new_dir = parent / options.new_folder_name;
			        std::filesystem::create_directory(new_dir);
			        options.is_app_options = options.is_create_folder = false;
			        options.new_folder_name[0] = '\0';
			    }
			}
            ImGui::Selectable("New file", false, 0, ImVec2(0, 40));
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) options.is_create_file = true;
            if (options.is_create_file) {
			    ImGui::SetKeyboardFocusHere();
			    if (ImGui::InputText("##new_file_name", options.new_file_name, 100, 
			        ImGuiInputTextFlags_EnterReturnsTrue)) {
			        auto parent = std::filesystem::path(options.clicked_path).parent_path();
			        auto new_file = parent / options.new_file_name;
			        std::ofstream(new_file).close();
			        options.is_app_options = options.is_create_file = false;
			        options.new_file_name[0] = '\0';
			    }
			    ImGui::SameLine();
			    if (ImGui::Button("Ok") && strlen(options.new_file_name) > 0) {
			        auto parent = std::filesystem::path(options.clicked_path).parent_path();
			        auto new_file = parent / options.new_file_name;
			        std::ofstream(new_file).close();
			        options.is_app_options = options.is_create_file = false;
			        options.new_file_name[0] = '\0';
			    }
			}
            ImGui::Selectable("Cut", false, 0, ImVec2(0, 40));
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
                options.copy_path = options.clicked_path;
                options.is_app_options = false;
                options.is_cut = true;
            }
            ImGui::Selectable("Copy", false, 0, ImVec2(0, 40));
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
                options.copy_path = options.clicked_path;
                options.is_app_options = false;
                options.is_cut = false;
            }
            ImGui::BeginDisabled(options.copy_path.empty() ? true : false);
            ImGui::Selectable("Paste", false, 0, ImVec2(0, 40));
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0) && options.copy_path != options.clicked_path) {
                if (std::filesystem::is_directory(options.copy_path)) {
                    std::system(("cp -r " + options.copy_path + " " + options.clicked_path).c_str());
                    if (options.is_cut) std::system(("rm -rf " + options.copy_path).c_str());
                } else {
                    std::system(("cp " + options.copy_path + " " + options.clicked_path).c_str());
                    if (options.is_cut) std::system(("rm " + options.copy_path).c_str());
                }
                options.copy_path = "";
                options.is_app_options = false;
                options.is_cut = false;
            }
            ImGui::EndDisabled();
            ImGui::Selectable("Rename", false, 0, ImVec2(0, 40));
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) options.is_rename = true;
            if (options.is_rename) {
                ImGui::InputText("##new_file_name", options.new_file_name, 100);
                ImGui::SameLine();
                if (ImGui::Button("Ok") && strcmp(options.new_file_name, "") != 0) {
                    std::system(("mv " + options.clicked_path + " " + options.clicked_path.substr(0, options.clicked_path.rfind(SLASH)) + SLASH + options.new_file_name).c_str());
                    options.is_app_options = false;
                    options.is_rename = false;
                    options.new_file_name[0] = '\0';
                }
            }
            ImGui::Selectable("Compress", false, 0, ImVec2(0, 40));
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
                std::system(("tar czf " + options.clicked_path + ".tar.gz " + options.clicked_path).c_str());
                options.is_app_options = false;
            }
            ImGui::Selectable("Remove", false, 0, ImVec2(0, 40));
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
                if (std::filesystem::is_directory(options.clicked_path)) std::system(("gio trash " + options.clicked_path).c_str());
                else std::system(("gio trash " + options.clicked_path).c_str());
                options.is_app_options = false;
            }
            ImGui::Selectable("Properties", false, 0, ImVec2(0, 40));
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
                if (std::filesystem::is_directory(options.clicked_path)) options.is_app_options = false;
            }
            ImGui::End();
        }
        if (options.is_dir_options) {
            if (options.width - options.mouse_pos.x < 200) options.mouse_pos.x -= 200;
            if (options.height - options.mouse_pos.y < 200) options.mouse_pos.y -= 200;
            if (ImGui::IsMouseClicked(0) && (ImGui::GetMousePos().x < options.mouse_pos.x || ImGui::GetMousePos().x > options.mouse_pos.x + 200 ||
                ImGui::GetMousePos().y < options.mouse_pos.y || ImGui::GetMousePos().y > options.mouse_pos.y + 200)) {
                options.is_dir_options = false;
            }
            ImGui::SetNextWindowPos(options.mouse_pos);
            ImGui::SetNextWindowSize(ImVec2(275, 220));
            ImGui::Begin("**options", &options.is_app_options, options_window_flags);
            ImGui::Selectable("New folder", false, 0, ImVec2(0, 40));
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))  options.is_create_folder = true;
            if (options.is_create_folder) {
			    ImGui::SetKeyboardFocusHere();
			    if (ImGui::InputText("##new_folder_name", options.new_folder_name, 100, 
			        ImGuiInputTextFlags_EnterReturnsTrue)) {
			        auto new_dir = std::filesystem::path(options.clicked_path) / options.new_folder_name;
			        std::filesystem::create_directory(new_dir);
			        options.is_dir_options = options.is_create_folder = false;
			        options.new_folder_name[0] = '\0';
			    }
			    ImGui::SameLine();
			    if (ImGui::Button("Ok") && strlen(options.new_folder_name) > 0) {
			        auto new_dir = std::filesystem::path(options.clicked_path) / options.new_folder_name;
			        std::filesystem::create_directory(new_dir);
			        options.is_dir_options = options.is_create_folder = false;
			        options.new_folder_name[0] = '\0';
			    }
			}
            ImGui::Selectable("New file", false, 0, ImVec2(0, 40));
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) options.is_create_file = true;
            if (options.is_create_file) {
			    ImGui::SetKeyboardFocusHere();
			    if (ImGui::InputText("##new_file_name", options.new_file_name, 100, 
			        ImGuiInputTextFlags_EnterReturnsTrue)) {
			        auto new_file = std::filesystem::path(options.clicked_path) / options.new_file_name;
			        std::ofstream(new_file).close();
			        options.is_dir_options = options.is_create_file = false;
			        options.new_file_name[0] = '\0';
			    }
			    ImGui::SameLine();
			    if (ImGui::Button("Ok") && strlen(options.new_file_name) > 0) {
			        auto new_file = std::filesystem::path(options.clicked_path) / options.new_file_name;
			        std::ofstream(new_file).close();
			        options.is_dir_options = options.is_create_file = false;
			        options.new_file_name[0] = '\0';
			    }
			}
            ImGui::BeginDisabled(options.copy_path.empty() ? true : false);
            ImGui::Selectable("Paste", false, 0, ImVec2(0, 40));
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0) && options.copy_path != options.clicked_path) {
                if (std::filesystem::is_directory(options.copy_path)) {
                    std::system(("cp -r " + options.copy_path + " " + options.clicked_path).c_str());
                    if (options.is_cut) std::system(("rm -rf " + options.copy_path).c_str());
                } else {
                    std::system(("cp " + options.copy_path + " " + options.clicked_path).c_str());
                    if (options.is_cut) std::system(("rm " + options.copy_path).c_str());
                }
                options.copy_path = "";
                options.is_dir_options = false;
                options.is_cut = false;
            }
            ImGui::EndDisabled();
            ImGui::Selectable("Properties", false, 0, ImVec2(0, 40));
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
                if (std::filesystem::is_directory(options.clicked_path)) options.is_dir_options = false;
            }
            ImGui::End();
        }
        if (options.is_menu_options) {
            if (options.width - options.mouse_pos.x < 210) options.mouse_pos.x -= 200;
            if (options.height - options.mouse_pos.y < 210) options.mouse_pos.y -= 200;
            ImGui::SetNextWindowPos(options.mouse_pos);
            ImGui::SetNextWindowSize(ImVec2(350, 80));
            ImGui::Begin("**menuoptions", &options.is_app_options, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
            ImGui::Checkbox("SHOW HIDDEN", &options.show_hidden);
            ImGui::End();
        }
        ImGui::EndChild();
    }
}
