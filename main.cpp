#include "main.h"

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Lab6 thingy", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
	
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
	io.IniFilename = nullptr;
    while (!glfwWindowShouldClose(window))
#endif
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        if (ImGui::Begin("Lab6", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
        {
			static bool enable_dumb_insert = false;
            static ImVector<ImVec2> points;
            static ImVec2 scrolling(0.0f, 0.0f);
            if (ImGui::Button("Back to (0, 0)")) scrolling = ImVec2{0.0f, 0.0f};
			
            static bool opt_enable_grid = true;
            static bool opt_enable_context_menu = true;
            static bool adding_point = false;
            static bool finding_best_step = false;
            static std::vector<int> current_shown_best_trace;
            static bool active_method_nearest_city = false;
            static std::vector<int> current_nearest_city_trace;
            static int current_start_from = 0;
            static int current_step = 0;
            static bool view_best_checkbox = true;
            static bool view_fast_checkbox = true;
            static bool view_nodes_checkbox = true;
            // Typically you would use a BeginChild()/EndChild() pair to benefit from a clipping region + own scrolling.
            // Here we demonstrate that this can be replaced by simple offsetting + custom drawing + PushClipRect/PopClipRect() calls.
            // To use a child window instead we could use, e.g:
            //      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));      // Disable padding
            //      ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255));  // Set a background color
            //      ImGui::BeginChild("canvas", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_NoMove);
            //      ImGui::PopStyleColor();
            //      ImGui::PopStyleVar();
            //      [...]
            //      ImGui::EndChild();

            // Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
            ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
            static int available_space_x = 400;
            canvas_sz.x -= available_space_x;
            if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
            if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
            ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

            // Draw border and background color
            ImGuiIO& io = ImGui::GetIO();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
            draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));
            bool get_best_trace = false;

            const ImVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
            const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);
            ImGui::SameLine();
            ImGui::Text((INTSTR((int)io.Framerate) + " FPS.").c_str());
            ImGui::SameLine();
			ImGui::Checkbox("Dumb insert", &enable_dumb_insert);
			ImGui::SameLine();
            if (ImGui::Button("canvas -")) available_space_x += 20;
            ImGui::SameLine();
            if (ImGui::Button("canvas +")) available_space_x -= 20;
            ImGui::SameLine();
            bool greedy_wants_next_step = false;
            if (finding_best_step) 
            {
                ImGui::SameLine();
                if (ImGui::Button("Next best trace"))
                {
                    std::vector<int> temp_trace = get_current_optimal_trace();
                    
                    if (temp_trace.size() == 0) 
                    {
                        is_optimal(0, true);
                        finding_best_step = false;
                    }
                    else
                    {
                        current_shown_best_trace = temp_trace;
                    }
                }
                if (finding_best_step) ImGui::BeginDisabled();
                ImGui::SameLine();
            }
            else
            if (active_method_nearest_city)
            {
                if (ImGui::Button("Next greedy step")) greedy_wants_next_step = true;
                ImGui::BeginDisabled(); ImGui::SameLine();
            }
            if (points.Size < 11 && ImGui::Button("Get best possible trace")) 
            {
                get_best_trace = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Get trace using greedy algorithm")) 
            {
                ImGui::BeginDisabled();
                active_method_nearest_city = true;
                current_start_from = randomnumber(0, points.Size - 1);
            }
            // This will catch our interactions
			if (finding_best_step || active_method_nearest_city) ImGui::EndDisabled();
            ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
            const bool is_hovered = ImGui::IsItemHovered(); // Hovered
            const bool is_active = ImGui::IsItemActive();   // Held
            if (is_hovered && !adding_point && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                adding_point = true;
            }
            if (adding_point)
            {
                if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
                {
					if (!active_method_nearest_city && !finding_best_step) 
					{
						points.push_back(mouse_pos_in_canvas);
						current_nearest_city_trace.clear();
						current_shown_best_trace.clear();
					}
                    adding_point = false;
                }
            }

            // Pan (we use a zero mouse threshold when there's no context menu)
            // You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
            const float mouse_threshold_for_pan = opt_enable_context_menu ? -1.0f : 0.0f;
            if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
            {
                scrolling.x += io.MouseDelta.x;
                scrolling.y += io.MouseDelta.y;
            }

            // Context menu (under default mouse threshold)
            ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
            if (opt_enable_context_menu && drag_delta.x == 0.0f && drag_delta.y == 0.0f && !active_method_nearest_city && !finding_best_step)
                ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
            if (!active_method_nearest_city && !finding_best_step && ImGui::BeginPopup("context"))
            {
                //if (adding_line)
                //    points.resize(points.size() - 2);
                //adding_line = false;
                if (ImGui::MenuItem("Remove all", NULL, false, points.Size > 0)) { points.clear(); current_nearest_city_trace.clear(); current_shown_best_trace.clear();}
                ImGui::EndPopup();
            }

            // Draw grid + all lines in the canvas
            std::vector<std::vector<int>> distance_for_method = std::vector<std::vector<int>>(points.Size, std::vector<int>(points.Size));
            draw_list->PushClipRect(canvas_p0, canvas_p1, true);
            if (opt_enable_grid)
            {
                const float GRID_STEP = 50.0f;
                draw_list->AddText(ImVec2(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y + GRID_STEP), IM_COL32(250, 0, 0, 250), "50");
                draw_list->AddText(ImVec2(canvas_p0.x + scrolling.x + GRID_STEP, canvas_p0.y + scrolling.y), IM_COL32(250, 0, 0, 250), "50"); 
                for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
                    if (scrolling.x - x == 0.0f) 
                    {
                        draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(250, 250, 250, 250));
                    } 
                    else
                    {
                        draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
                    }
                for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
                    if (scrolling.y - y == 0.0f)
                    {
                        draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(250, 250, 250, 250));
                    }
                    else
                    {
                        draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
                    }
            }
            bool hover_cleared = false;
			std::string hover_tooltip_text;
            //ImGui::SetNextWindowPos(ImVec2(scrolling.x, scrolling.y));
            for (int n = 0; n < points.Size; n += 1)
            {
                distance_for_method[n][n] = 0;
                float min_dist_to_mouse = __FLT_MAX__;
                draw_list->AddCircleFilled(ImVec2(ImVec2(origin.x + points[n].x, origin.y + points[n].y)), 6, IM_COL32(255, 255, 0, 255), 4);
                for (int j = 0; j < n; j++)
                {
                    float x1 = points[n].x + origin.x; float y1 = points[n].y + origin.y;
                    float x2 = points[j].x + origin.x; float y2 = points[j].y + origin.y;
                    float x3 = mouse_pos_in_canvas.x + origin.x; float y3 = mouse_pos_in_canvas.y + origin.y;
                    float dist = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
                    float A = (y2-y1)*(y2-y1) + (x1-x2)*(x1-x2);
                    float B = (y2-y1)*(x1*y2 - x2*y1);
                    float C = (x1-x2)*(x1-x2)*x3;
                    float D = y3*(x2-x1)*(y2-y1);
                    float x0 = A == 0 ? 0 : ( B + C + D ) / A; //нормаль к полученной прямой, точка соприкосновения.
                    float y0 = (x1*y2 - x2*y1 - x0*(y2-y1))/(x1-x2);
                    bool is_on_line = (x2 - x0) * (x1 - x0) < 0;
                    float A1 = points[j].y - points[n].y;
                    float B1 = points[n].x - points[j].x;
                    float C1 = points[j].x * points[n].y - points[n].x * points[j].y;
                    float d = (A1 * mouse_pos_in_canvas.x + B1 * mouse_pos_in_canvas.y + C1) / (sqrt(A1*A1 + B1*B1)); //расстояние до полученной прямой
                    bool found_actual_trail = false;
                    if (current_shown_best_trace.size() != 0)
                    {
                        for (int k = 0; k < current_shown_best_trace.size() - 1; k++)
                        {
							if (!view_best_checkbox)
							{
								found_actual_trail = true;
								break;
							}
                            if ((current_shown_best_trace[k] == n && current_shown_best_trace[k + 1] == j) || (current_shown_best_trace[k] == j && current_shown_best_trace[k + 1] == n))
                            {
                                found_actual_trail = true;
                                draw_list->AddLine(ImVec2(origin.x + points[n].x, origin.y + points[n].y), ImVec2(origin.x + points[j].x, origin.y + points[j].y), IM_COL32(0, 255, 0, 255), 5.0f);
                                break;
                            }
                        }
                        if (view_best_checkbox && current_shown_best_trace.size() > 2 && ((current_shown_best_trace[0] == n && current_shown_best_trace[current_shown_best_trace.size() - 1] == j) || (current_shown_best_trace[0] == j && current_shown_best_trace[current_shown_best_trace.size() - 1] == n)))
                        {
                            draw_list->AddLine(ImVec2(origin.x + points[n].x, origin.y + points[n].y), ImVec2(origin.x + points[j].x, origin.y + points[j].y), IM_COL32(0, 155, 0, 255), 5.0f);
                        }
                    }
                    if (current_nearest_city_trace.size() != 0)
                    {
                        for (int k = 0; k < current_nearest_city_trace.size() - 1; k++)
                        {
							if (!view_fast_checkbox)
							{
								found_actual_trail = true;
								break;
							}
                            if ((current_nearest_city_trace[k] == n && current_nearest_city_trace[k + 1] == j) || (current_nearest_city_trace[k] == j && current_nearest_city_trace[k + 1] == n))
                            {
                                found_actual_trail = true;
                                draw_list->AddLine(ImVec2(origin.x + points[n].x, origin.y + points[n].y), ImVec2(origin.x + points[j].x, origin.y + points[j].y), IM_COL32(255, 255, 0, 255), 3.0f);
                                break;
                            }
                        }
                        if (view_fast_checkbox && current_nearest_city_trace.size() > 2 && ((current_nearest_city_trace[0] == n && current_nearest_city_trace[current_nearest_city_trace.size() - 1] == j) || (current_nearest_city_trace[0] == j && current_nearest_city_trace[current_nearest_city_trace.size() - 1] == n)))
                        {
                            draw_list->AddLine(ImVec2(origin.x + points[n].x, origin.y + points[n].y), ImVec2(origin.x + points[j].x, origin.y + points[j].y), IM_COL32(255, 155, 0, 255), 3.0f);
                        }
                    }
                    if (20 > abs(d) && is_on_line && !found_actual_trail)
                    {
						if (hover_cleared) hover_tooltip_text+="\n";
						hover_tooltip_text+=INTSTR(dist);
                        hover_cleared = true;
                        
                        draw_list->AddLine(ImVec2(origin.x + points[n].x, origin.y + points[n].y), ImVec2(origin.x + points[j].x, origin.y + points[j].y), IM_COL32(100, 0, 0, 100), 2.0f);
                    }
                    else if (view_nodes_checkbox)
                    {
                        draw_list->AddLine(ImVec2(origin.x + points[n].x, origin.y + points[n].y), ImVec2(origin.x + points[j].x, origin.y + points[j].y), IM_COL32(100, 100, 0, 100), 2.0f);
                    }
                    distance_for_method[n][j] = dist;
                    distance_for_method[j][n] = dist;
                }
            }
			if (hover_cleared)
			{
				ImGui::SetTooltip(hover_tooltip_text.c_str());
			}
            for (int n = 0; n < points.Size; n++)
            {
                draw_list->AddText(ImVec2(origin.x + points[n].x + 2, origin.y + points[n].y), IM_COL32(255, 255, 255, 255), INTSTR(n + 1).c_str());
            }
            draw_list->PopClipRect();
            if (greedy_wants_next_step)
            {
                greedy_wants_next_step = false;
                current_step++;
                std::vector<int> temp_ = method(points.Size, distance_for_method, current_step, current_start_from, enable_dumb_insert);
                if (temp_.size() == 0) 
                {
                    active_method_nearest_city = false;
                    current_step = 0;
                }
                else current_nearest_city_trace = temp_;
                //if (!active_method_nearest_city) ImGui::EndDisabled();
            }
            ImGui::SameLine();
            ImGui::BeginGroup();
            if (points.Size != 0 && ImGui::BeginTable("Current matrix", points.Size + 1, ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedSame))
            {
                for (int i = 0; i < points.Size + 1; i++)
                {
                    ImGui::TableNextColumn();
                    ImGui::TextDisabled(INTSTR(i).c_str());
                }
                for (int i = 0; i < points.Size; i++)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextDisabled(INTSTR(i + 1).c_str());
                    for (int j = 0; j < points.Size; j++)
                    {
                        ImGui::TableSetColumnIndex(j + 1);
                        ImGui::Text(INTSTR(distance_for_method[i][j]).c_str());
                    }
                }
                ImGui::EndTable();
            }
            
            
            int current_best = 0;
            if (current_shown_best_trace.size()!=0)
            {
                for (int i = 0; i < (current_shown_best_trace.size() - 1); i++)
                {
                    current_best += distance_for_method[current_shown_best_trace[i]][current_shown_best_trace[i+1]];
                }
                current_best += distance_for_method[current_shown_best_trace[0]][current_shown_best_trace[current_shown_best_trace.size() - 1]];
            }
            int current_method_best = 0;
            if (current_nearest_city_trace.size()!=0)
            {
                for (int i = 0; i < (current_nearest_city_trace.size() - 1); i++)
                {
                    current_method_best += distance_for_method[current_nearest_city_trace[i]][current_nearest_city_trace[i+1]];
                }
                current_method_best += distance_for_method[current_nearest_city_trace[0]][current_nearest_city_trace[current_nearest_city_trace.size() - 1]];
            }
            ImGui::TextWrapped(("Current best trace is " + INTSTR(current_best)).c_str());
            ImGui::TextWrapped(("Current greedy trace is " + INTSTR(current_method_best)).c_str());
            
			ImGui::Separator();
			ImGui::Checkbox("show best nodes", &view_best_checkbox);
            ImGui::Checkbox("show greedy nodes", &view_fast_checkbox);
            ImGui::Checkbox("show all nodes", &view_nodes_checkbox);
			ImGui::Separator();
            if (get_best_trace)
            {
                finding_best_step = true;
                print_optimal(distance_for_method);
            }
			ImGui::TextWrapped("right click: move and delete all button");
			ImGui::TextWrapped("left click: create points");
            ImGui::EndGroup();
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
