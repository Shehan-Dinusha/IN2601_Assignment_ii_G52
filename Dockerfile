FROM ubuntu:22.04

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    g++-mingw-w64-x86-64 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src

ENTRYPOINT ["sh", "-c"]
CMD ["x86_64-w64-mingw32-g++ -g -std=c++17 -Iinclude -Ivendor/glad -Ivendor/glfw -Ivendor/imgui -Llib src/*.cpp vendor/glad/glad.c vendor/imgui/*.cpp -lglfw3dll -lopengl32 -lglu32 -o app.exe"]
