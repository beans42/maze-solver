OUT_DIR = out

SOURCES = main.cpp
SOURCES += imgui/imgui_impl_glfw.cpp imgui/imgui_impl_opengl3.cpp
SOURCES += imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_demo.cpp imgui/imgui_widgets.cpp imgui/imgui_tables.cpp

EMCC_OPTS = -s WASM=1
EMCC_OPTS += -s USE_WEBGL2=1 -s USE_GLFW=3 -s FULL_ES3=1
EMCC_OPTS += -s EXTRA_EXPORTED_RUNTIME_METHODS=['ccall']
EMCC_OPTS += -s EXPORTED_FUNCTIONS="['_main', '_update_file']"
EMCC_OPTS += -s ALLOW_MEMORY_GROWTH=1

all: $(SOURCES) 
	emcc -o imgui.js -Iimgui $(SOURCES) -lGL $(EMCC_OPTS)
	rm -rf $(OUT_DIR)/*
	mv imgui.js $(OUT_DIR)/imgui.js
	mv imgui.wasm $(OUT_DIR)/imgui.wasm
	cp index.html $(OUT_DIR)/index.html

clean:
	rm -f $(OUT_DIR)/imgui.js $(OUT_DIR)/imgui.wasm $(OUT_DIR)/index.html