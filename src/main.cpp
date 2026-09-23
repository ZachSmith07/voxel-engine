#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "graphics/Shader.h"
#include "graphics/Camera.h"
#include "graphics/objects/Cube.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "graphics/stb_image.h"
#include "world/World.h"
#include "graphics/CrosshairRenderer.h"
#include "graphics/DebugBlockRay.h"
#include "graphics/DebugRenderer.h"
#include "world/FastNoiseLite.h"
#include "player/Player.h"
#include "basics/stb_easy_font.h"
#include "world/DroppedItem.h"
#include <freetype-gl.h>
#include <text-buffer.h>
#include <vertex-buffer.h>
#include <markup.h>
#include "basics/TextRenderer.h"
#include "player/ItemDatabase.h"
#include "player/Crafting.h"
#include "ui/Basics.h"
#include "ui/Hotbar.h"
#include "ui/Inventory.h"

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
float lastX = 400.0f, lastY = 300.0f;
bool firstMouse = true;
float deltaTime = 0.0f, lastFrame = 0.0f;
bool inventoryOpen = false;

// === Global texture atlas ID ===
unsigned int atlasTexture;
unsigned int textureID;

void LoadTexture(const std::string &path = "../../assets/textures/FullMap.png")
{
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);

    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "SUCCESSFULLY LOADED FullMap.png" << std::endl;
        std::cout << width << ", " << height << std::endl;
        std::cout << textureID << " - textureID" << std::endl;
    }
    else
    {
        std::cout << "FAILED TO LOAD" << std::endl;
        std::cerr << "Failed to load texture atlas: " << path << std::endl;
    }

    stbi_image_free(data);
}

// === Load texture atlas from file ===
void LoadTextureAtlas()
{
    glGenTextures(1, &atlasTexture);
    glBindTexture(GL_TEXTURE_2D, atlasTexture);

    // Nearest filter for pixel-perfect voxel look
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load("../../assets/textures/textures.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "Texture ID after load: " << textureID << std::endl;
    }
    else
    {
        std::cerr << "Failed to load texture atlas\n";
    }
    std::cout << "Loaded texture: " << width << "x" << height << std::endl;
    stbi_image_free(data);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    if (!inventoryOpen)
        camera.ProcessMouseMovement(xoffset, yoffset);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void RenderText(const char *text, float x, float y, float r, float g, float b)
{
    char buffer[99999]; // ~500 chars
    int num_quads = stb_easy_font_print(x, y, (char *)text, nullptr, buffer, sizeof(buffer));

    glColor3f(r, g, b);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 16, buffer);
    glDrawArrays(GL_QUADS, 0, num_quads * 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}

int main()
{
    if (!glfwInit())
    {
        std::cout << "INIT" << std::endl;
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    else
    {
        std::cout << "INIT" << std::endl;
    }

    GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(primaryMonitor);

    GLFWwindow *window = glfwCreateWindow(
        mode->width, mode->height,
        "Voxel Engine",
        primaryMonitor, // ← This makes it fullscreen
        nullptr);

    glfwMakeContextCurrent(window);

    // ✅ Initialize GLEW instead of GLAD
    glewExperimental = GL_TRUE; // Important for core profiles
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW\n";
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);

    LoadTextureAtlas();
    LoadTexture();

    Shader shader("../../shaders/basic.vert", "../../shaders/basic.frag");
    Shader itemsShader("../../shaders/basic.vert", "../../shaders/basic.frag");
    CrosshairRenderer crosshair;
    Shader shader2("../../shaders/crosshair.vert", "../../shaders/crosshair.frag");
    Shader debugShader("../../shaders/basic.vert", "../../shaders/basic.frag");

    World world;
    DebugRenderer debug;
    debug.Init();

    Player player = Player({});
    camera.MovementSpeed = player.playerStats.CalculateMovementSpeed();

    double lastTime = glfwGetTime();
    int nbFrames = 0;

    float lastIndexChange = 0.0f;

    Shader textShader("../../shaders/text.vert", "../../shaders/text.frag");
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(mode->width), 0.0f, static_cast<float>(mode->height));
    textShader.Bind();
    textShader.SetUniformMat4("projection", glm::value_ptr(projection));
    textShader.Unbind();

    TextRenderer textRenderer(mode->width, mode->height, textShader);
    textRenderer.Load("../../assets/fonts/arial.ttf", 48);

    bool iDown = false;
    int currentSlotClicked = -1;
    bool isLeft = true;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Bind();
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 360.0f);
        shader.SetUniformMat4("u_ViewProj", glm::value_ptr(proj * view));

        world.Update(camera);
        world.Draw(shader);

        auto blockPos = world.GetTargetBlock(camera.Position - glm::vec3(0.0f, 1.0f, 0.0f), camera.GetDirectionVector());
        if (blockPos)
        {
            shader.Unbind();
            debugShader.Bind();
            debugShader.SetUniformMat4("u_ViewProj", glm::value_ptr(proj * view));
            Cube debugCube2(*blockPos + glm::vec3(0.5f, 1.5f, 0.5f), 2.0f);
            debugCube2.Draw(debugShader);
            debugShader.Unbind();
        }
        else
        {
            Cube debugCube(glm::vec3(0.0f, 1.0f, 0.0f), 2);
            debugCube.Draw(shader);
            shader.Unbind();
        }

        auto start = std::chrono::high_resolution_clock::now();
        itemsShader.Bind();
        itemsShader.SetUniformMat4("u_ViewProj", glm::value_ptr(proj * view));
        world.DrawDroppedItems(itemsShader, camera);
        itemsShader.Unbind();
        std::vector<DroppedItem> itemPickups = world.pickupItems(camera);
        for (DroppedItem item : itemPickups)
        {
            player.AddItem({item.quantity, item.itemID});
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        crosshair.Draw(shader2);

        glm::mat4 viewProj = camera.GetProjection() * camera.GetViewMatrix();
        debug.Render(viewProj);
        debug.Clear();

        static bool canClick = true;

        if (inventoryOpen)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        {
            if (!iDown)
            {
                inventoryOpen = !inventoryOpen;

                iDown = true;
            }
        }
        else if (glfwGetKey(window, GLFW_KEY_I) == GLFW_RELEASE)
        {
            iDown = false;
        }

        if (!inventoryOpen)
        {
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            {
                if (glfwGetTime() - 0.1f > lastIndexChange)
                {
                    if (player.hotbarIndex > 0)
                    {
                        player.hotbarIndex -= 1;
                    }
                    lastIndexChange = glfwGetTime();
                }
            }
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            {
                if (glfwGetTime() - 0.1f > lastIndexChange)
                {
                    if (player.hotbarIndex < 7)
                    {
                        player.hotbarIndex += 1;
                    }
                    lastIndexChange = glfwGetTime();
                }
            }

            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && canClick)
            {
                auto blockPos = world.GetTargetBlock(camera.Position - glm::vec3(0.0f, 1.0f, 0.0f), camera.GetDirectionVector());
                if (blockPos)
                {
                    glm::vec3 pos = *blockPos;
                    world.SetBlock(pos.x, pos.y, pos.z, 0);
                    canClick = false;
                }
            }
            else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && canClick)
            {
                auto blockPos = world.GetTargetBlock(camera.Position - glm::vec3(0.0f, 1.0f, 0.0f), camera.GetDirectionVector(), true);
                if (blockPos)
                {
                    glm::vec3 pos = *blockPos;
                    world.SetBlock(pos.x, pos.y, pos.z, player.PlaceItem(), false);
                    canClick = false;
                }
            }
            else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE &&
                     glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
            {
                canClick = true;
            }
        }

        nbFrames++;
        double currentTime = glfwGetTime();
        if (currentTime - lastTime >= 1.0)
        {
            nbFrames = 0;
            lastTime = currentTime;
        }

        glDisable(GL_DEPTH_TEST); // ✏️ turn off depth test so text is on top

        // TEXT DISPLAY

        double fps = nbFrames / (currentTime - lastTime);
        std::string text = std::to_string(int(fps)) + "FPS";
        textRenderer.RenderText(text, 25.0f, mode->height - 50.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

        std::vector<std::string> strings = player.GetItemsAsStrings();
        for (int i = 0; i < strings.size(); ++i)
        {
            textRenderer.RenderText(strings[i], 25.0f, 125.0f + i * 65.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
        }

        glUseProgram(0);

        DrawHotbar(mode->width, mode->height, player.hotbar, textRenderer, player.hotbarIndex);
        if (inventoryOpen)
        {
            // INVENTORY CLICKING

            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && canClick)
            {
                double mouseX, mouseY;
                glfwGetCursorPos(window, &mouseX, &mouseY);

                std::cout << "X: " << mouseX << ", Y: " << mouseY << std::endl;

                int slot = GetInventorySlotFromMouse(mouseX, mode->height - mouseY, mode->width, mode->height);
                if (slot == 36)
                {
                    if (currentSlotClicked == 36)
                    {
                        std::optional<Item> item = CalculateItem(player.crafting);
                        if (item)
                        {
                            if (player.craftedItem)
                            {
                                Item craftedItem = *player.craftedItem;
                                if (craftedItem.itemID == (*item).itemID)
                                {
                                    int maxStack = ITEMS[craftedItem.itemID].maxStack;
                                    if (craftedItem.quantity + (*item).quantity <= maxStack)
                                    {
                                        Item newItem = {craftedItem.quantity + (*item).quantity, craftedItem.itemID};
                                        player.craftedItem = newItem;
                                        player.RemoveCraftingLayer();
                                    }
                                }
                            }
                            else
                            {
                                player.craftedItem = item;
                                player.RemoveCraftingLayer();
                            }
                        }
                    }
                    currentSlotClicked = slot;
                }
                else
                {
                    if (currentSlotClicked == slot)
                    {
                        isLeft = true;
                        currentSlotClicked = -1;
                    }
                    else
                    {
                        if (isLeft)
                        {
                            if (currentSlotClicked != -1 && slot != -1)
                            {
                                player.SwapItems(currentSlotClicked, slot);
                            }
                            currentSlotClicked = slot;
                        }
                        else
                        {
                            if (currentSlotClicked != -1 && slot != -1 && currentSlotClicked != 36 && slot != 36)
                            {
                                player.AddItemSpecific(currentSlotClicked, slot);
                            }
                        }
                    }
                }
                if (slot == -1)
                {
                    currentSlotClicked = -1;
                    isLeft = true;
                }
                std::cout << slot << std::endl;

                canClick = false;
            }
            else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
            {
                canClick = true;
            }

            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && canClick)
            {
                double mouseX, mouseY;
                glfwGetCursorPos(window, &mouseX, &mouseY);

                std::cout << "X: " << mouseX << ", Y: " << mouseY << std::endl;

                int slot = GetInventorySlotFromMouse(mouseX, mode->height - mouseY, mode->width, mode->height);
                currentSlotClicked = slot;
                isLeft = false;
            }
            else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
            {
            }

            DrawInventory(mode->width, mode->height, player.hotbar, player.inventory, player.crafting, player.craftedItem, textRenderer, currentSlotClicked);
        }

        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}