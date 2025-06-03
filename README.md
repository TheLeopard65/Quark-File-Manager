# 🗂️ QUARK File Manager

**A sleek, modern, cross-platform file manager built with C++, OpenGL, and ImGui.**
Designed for developers and power users who crave speed, customization, and minimalist UI.

`NOTE` This is my Fourth Semeseter Operating System Course Project. Project & Quality isn't Great.

---

## 🚀 Features

* 📂 Tree-style file navigation
* 🧭 Bookmark quick-access panel (e.g., Home, Desktop, Downloads)
* 🔍 Hidden file visibility toggle
* 🧠 Keyboard shortcuts (e.g., `ESC` to exit)
* 📑 Detailed file info: name, size, permissions, type, last modified
* 🧰 Context menus with:

  * Open file/folder
  * Create new file/folder
  * Cut, copy, paste
  * Rename
  * Compress (.tar.gz)
  * Trash/delete

* 🎨 Custom theming with `ImGui` and `Courier-New` fonts
* 🔌 Cross-platform support: **Linux & Windows**

---

## 🛠️ Build Instructions

### ✅ Prerequisites

#### Linux:

Make sure the following packages are installed:

```bash
sudo apt install -y cmake libxrandr-dev libx11-dev libxext-dev libxi-dev libgl1-mesa-dev libxinerama-dev libxcursor-dev
```

#### Windows:

* Visual Studio (2019 or later)
* CMake
* OpenGL & GLFW development libraries

---

### 🔧 Building the Project

```bash
git clone https://github.com/TheLeopard65/Quark-File-Manager.git
cd Quark-File-Manager
mkdir build && cd build
cmake ..
make -j$(nproc)
./QUARK
```

> 🔄 On Windows, open the generated `.sln` in Visual Studio and build the `QUARK` target.

---

## 📁 Project Structure

```
File-Manager/
├── src/                    # Main C++ source file
│   └── main.cpp
├── vendor/                 # Third-party dependencies (GLFW, ImGui, stb)
├── resources/              # Icons and fonts used in the UI
│   ├── folder.png
│   ├── file.png
│   ├── go_back.png
│   └── Courier-New.ttf
├── requirements.txt        # Linux build dependencies
├── CMakeLists.txt          # Build configuration
└── README.md               # You're here!
```

---

## 💡 Technical Highlights

* Built with **C++17**
* Uses **ImGui** for rendering custom GUI
* Renders textures with **OpenGL**
* Efficient filesystem browsing using `<filesystem>`
* Platform-specific integrations for Linux (`xdg-open`) and Windows (`start`)

---

## 👨‍💻 Contributin

Pull requests are welcome! Please fork the repository and submit your changes via a new branch.

---

## ⚖️ License

The repo/project is under the MIT License. See [`LICENSE`](LICENSE) for details.

---

## 🙌 Credits

* [Dear ImGui](https://github.com/ocornut/imgui)
* [GLFW](https://github.com/glfw/glfw)
* [stb](https://github.com/nothings/stb)

---

## 💬 Feedback

Found a bug? Want to suggest a feature?
Open an [issue](https://github.com/your-username/Quark-File-Manager/issues).
