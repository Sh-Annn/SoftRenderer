由于 src/app.cpp 中使用了相对路径 ../obj/... 来加载资源，这意味着可执行文件运行时，必须位于一个名为 obj 的文件夹的同级目录的子目录中（例如：Root/bin/app 和 Root/obj，此时 ../obj 指向
  Root/obj）。

  在“不修改源代码（app.cpp）”的前提下，要实现生成可执行文件后自动包含 obj 资源并能正确运行，主要有以下几种方案：

  方案一：修改 CMake 构建脚本（推荐）

  通过修改 CMakeLists.txt，我们可以调整可执行文件的输出位置，并添加构建后自动复制资源的命令。这样每次构建后，build 目录就是一个自包含的、可直接分发的包。

  原理：
   1. 将可执行文件输出到 build/bin 目录。
   2. 将 obj 文件夹复制到 build/obj 目录。
   3. 这样可执行文件在 bin 中运行，../obj 就能正确找到 build/obj。

  操作步骤：
  在 CMakeLists.txt 的末尾（add_executable 之后）添加以下内容：

    1 # 1. 设置可执行文件的输出目录为构建目录下的 bin 子文件夹
    2 set_target_properties(soft_renderer PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    3
    4 # 2. 添加构建后命令：将源代码根目录的 obj 文件夹复制到构建目录根目录
    5 add_custom_command(TARGET soft_renderer POST_BUILD
    6     COMMAND ${CMAKE_COMMAND} -E copy_directory
    7     "${CMAKE_SOURCE_DIR}/obj"
    8     "${CMAKE_BINARY_DIR}/obj"
    9     COMMENT "Copying assets to build directory..."
   10 )

  运行方式：
  构建完成后，进入 build/bin 目录运行：
   1 cd build/bin
   2 ./soft_renderer

  ---

  方案二：使用发布脚本（完全不修改任何项目文件）

  如果你严格限制不能修改 CMakeLists.txt，可以编写一个简单的 Shell 脚本来完成“打包”工作。

  原理：
  手动创建一个符合 ../obj 路径结构的文件夹结构（例如 dist 目录）。

  操作步骤：
  在项目根目录创建一个脚本（如 package.sh）或直接运行以下命令：

    1 # 创建发布目录结构
    2 mkdir -p dist/bin
    3
    4 # 复制可执行文件到 bin 目录
    5 cp build/soft_renderer dist/bin/
    6
    7 # 复制资源文件夹到 dist 根目录
    8 cp -r obj dist/
    9
   10 echo "打包完成。运行方式：cd dist/bin && ./soft_renderer"

  运行方式：
   1 cd dist/bin
   2 ./soft_renderer

  ---

  方案三：开发时使用软链接（仅限 Linux/macOS）

  如果在开发过程中不想每次都复制文件，可以在构建目录中创建软链接。

  原理：
  模拟方案一的目录结构，通过软链接指向源码目录。

  操作步骤：

   1 # 假设你在 build 目录中
   2 mkdir -p bin
   3 mv soft_renderer bin/  # 移动可执行文件到子目录
   4 ln -s ../../obj obj    # 在 build 根目录创建指向源码 obj 的软链接

  总结
   * 最推荐方案一：它将资源复制和路径结构集成到了构建系统中，使得 build 目录下的产物是完整的、可移植的。
   * 最严格方案二：完全不触碰项目文件，通过外部命令重组目录结构。

  如果您希望我为您应用方案一（修改 CMakeLists.txt），请告诉我。
