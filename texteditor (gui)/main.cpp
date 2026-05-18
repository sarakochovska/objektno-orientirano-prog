#include "raylib.h"
#include <cstring>
#include <fstream>
using namespace std;

void SaveFile(const char* path, const char* data)
{
    ofstream out(path);
    if (out.is_open()) out << data;
}

void LoadFile(const char* path, char* data, int maxLen)
{
    ifstream in(path);
    if (!in.is_open()) return;
    in.read(data, maxLen - 1);
    data[in.gcount()] = '\0';
}

string OpenFileDialog()
{
    FILE* stream = popen("zenity --file-selection --file-filter='Text files | *.txt'", "r");
    if (!stream) return "";
    char buffer[1024];
    string pathStr;
    if (fgets(buffer, sizeof(buffer), stream))
    {
        pathStr = buffer;
        if (!pathStr.empty() && pathStr.back() == '\n') pathStr.pop_back();
    }
    pclose(stream);
    return pathStr;
}

string SaveFileDialog()
{
    FILE* stream = popen("zenity --file-selection --save --confirm-overwrite --filename=file.txt --file-filter='Text files | *.txt'", "r");
    if (!stream) return "";
    char buffer[1024];
    string pathStr;
    if (fgets(buffer, sizeof(buffer), stream))
    {
        pathStr = buffer;
        if (!pathStr.empty() && pathStr.back() == '\n') pathStr.pop_back();
    }
    pclose(stream);
    if (!pathStr.empty())
    {
        if (pathStr.size() < 4 || pathStr.substr(pathStr.size() - 4) != ".txt") pathStr += ".txt";
    }
    return pathStr;
}

int main()
{
    const int wWidth = 960;
    const int wHeight = 560;
    InitWindow(wWidth, wHeight, "custom text editor");
    SetTargetFPS(60);

    char txtBuffer[1024] = "Type here!";
    int caretIdx = strlen(txtBuffer);
    float scrollOffset = 0.0f;

    while (!WindowShouldClose())
    {
        int strLength = strlen(txtBuffer);
        float frameTime = GetFrameTime();
        int rowLength = 0;

        for (int i = strLength - 1; i >= 0; i--)
        {
            if (txtBuffer[i] == '\n') break;
            rowLength++;
        }

        int inputKey = GetCharPressed();
        while (inputKey > 0)
        {
            int currentLen = strlen(txtBuffer);
            int rowStart = caretIdx;
            for (int i = caretIdx - 1; i >= 0; i--)
            {
                if (txtBuffer[i] == '\n') break;
                rowStart = i;
            }
            int activeRowLen = 0;
            for (int i = rowStart; i < currentLen; i++)
            {
                if (txtBuffer[i] == '\n') break;
                activeRowLen++;
            }
            if (activeRowLen >= 80 && inputKey != '\n')
            {
                inputKey = GetCharPressed();
                continue;
            }
            if (currentLen < 1023)
            {
                for (int i = currentLen; i >= caretIdx; i--) txtBuffer[i + 1] = txtBuffer[i];
                txtBuffer[caretIdx] = (char)inputKey;
                caretIdx++;
                txtBuffer[currentLen + 1] = '\0';
            }
            inputKey = GetCharPressed();
        }

        static float bsTimer = 0.0f;
        static bool bsFirst = true;
        if (IsKeyDown(KEY_BACKSPACE))
        {
            bsTimer += GetFrameTime();
            float delay = bsFirst ? 0.0f : 0.05f;
            if (bsTimer >= delay)
            {
                int currentLen = strlen(txtBuffer);
                if (caretIdx > 0)
                {
                    for (int i = caretIdx - 1; i < currentLen; i++) txtBuffer[i] = txtBuffer[i + 1];
                    caretIdx--;
                }
                bsTimer = 0.0f;
                bsFirst = false;
            }
        }
        else
        {
            bsTimer = 0.0f;
            bsFirst = true;
        }

        static float entTimer = 0.0f;
        static bool entFirst = true;
        if (IsKeyDown(KEY_ENTER))
        {
            float delay = entFirst ? 0.0f : 0.12f;
            entTimer += GetFrameTime();
            if (entTimer >= delay)
            {
                int currentLen = strlen(txtBuffer);
                if (currentLen < 1023 && caretIdx <= currentLen)
                {
                    for (int i = currentLen; i >= caretIdx; i--) txtBuffer[i + 1] = txtBuffer[i];
                    txtBuffer[caretIdx] = '\n';
                    caretIdx++;
                    txtBuffer[currentLen + 1] = '\0';
                }
                entTimer = 0.0f;
                entFirst = false;
            }
        }
        else
        {
            entTimer = 0.0f;
            entFirst = true;
        }

        static float navTimer = 0.0f;
        static bool navFirst = true;
        bool pressLeft = IsKeyDown(KEY_LEFT);
        bool pressRight = IsKeyDown(KEY_RIGHT);
        if (pressLeft || pressRight)
        {
            float delay = navFirst ? 0.0f : 0.05f;
            navTimer += GetFrameTime();
            if (navTimer >= delay)
            {
                if (pressLeft && caretIdx > 0) caretIdx--;
                if (pressRight && caretIdx < (int)strlen(txtBuffer)) caretIdx++;
                navTimer = 0.0f;
                navFirst = false;
            }
        }
        else
        {
            navTimer = 0.0f;
            navFirst = true;
        }

        static float rowTimer = 0.0f;
        static bool rowFirst = true;
        bool pressUp = IsKeyDown(KEY_UP);
        bool pressDown = IsKeyDown(KEY_DOWN);
        if (pressUp || pressDown)
        {
            float delay = rowFirst ? 0.0f : 0.08f;
            rowTimer += GetFrameTime();
            if (rowTimer >= delay)
            {
                int lineStartIdx = 0;
                int lineCounter = 0;
                for (int i = 0; i < caretIdx; i++)
                {
                    if (txtBuffer[i] == '\n')
                    {
                        lineStartIdx = i + 1;
                        lineCounter++;
                    }
                }
                int currentColumn = caretIdx - lineStartIdx;
                if (pressUp)
                {
                    if (lineCounter > 0)
                    {
                        int targetLineStart = 0;
                        int trackLine = 0;
                        for (int i = 0; i < caretIdx; i++)
                        {
                            if (txtBuffer[i] == '\n')
                            {
                                if (trackLine == lineCounter - 1) break;
                                targetLineStart = i + 1;
                                trackLine++;
                            }
                        }
                        int targetPos = targetLineStart + currentColumn;
                        int lineLimit = targetLineStart;
                        while (txtBuffer[lineLimit] != '\0' && txtBuffer[lineLimit] != '\n') lineLimit++;
                        if (targetPos > lineLimit) targetPos = lineLimit;
                        caretIdx = targetPos;
                    }
                }
                if (pressDown)
                {
                    int trackLine = 0;
                    int nextLineStartIdx = -1;
                    for (int i = 0; txtBuffer[i] != '\0'; i++)
                    {
                        if (txtBuffer[i] == '\n')
                        {
                            if (trackLine == lineCounter) nextLineStartIdx = i + 1;
                            trackLine++;
                        }
                    }
                    if (nextLineStartIdx != -1)
                    {
                        int targetPos = nextLineStartIdx + currentColumn;
                        int lineLimit = nextLineStartIdx;
                        while (txtBuffer[lineLimit] != '\0' && txtBuffer[lineLimit] != '\n') lineLimit++;
                        if (targetPos > lineLimit) targetPos = lineLimit;
                        caretIdx = targetPos;
                    }
                }
                rowTimer = 0.0f;
                rowFirst = false;
            }
        }
        else
        {
            rowTimer = 0.0f;
            rowFirst = true;
        }

        scrollOffset -= GetMouseWheelMove() * 50.0f;
        if (scrollOffset < 0) scrollOffset = 0;

        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S))
        {
            string fileSelection = SaveFileDialog();
            if (!fileSelection.empty()) SaveFile(fileSelection.c_str(), txtBuffer);
        }
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_O))
        {
            string fileSelection = OpenFileDialog();
            if (!fileSelection.empty())
            {
                LoadFile(fileSelection.c_str(), txtBuffer, 1024);
                caretIdx = strlen(txtBuffer);
            }
        }

        int activeLine = 0;
        int activeCol = 0;
        for (int i = 0; i < caretIdx; i++)
        {
            if (txtBuffer[i] == '\n')
            {
                activeLine++;
                activeCol = 0;
            }
            else
            {
                activeCol++;
            }
        }

        BeginDrawing();
        ClearBackground(WHITE);
        DrawRectangle(10, 25, 940, 492, WHITE);
        DrawTextEx(GetFontDefault(), txtBuffer, (Vector2){15, 30 - scrollOffset}, 20, 1, BLACK);

        int footerHeight = 25;
        DrawRectangle(0, wHeight - footerHeight, wWidth, footerHeight, LIGHTGRAY);
        DrawLine(0, wHeight - footerHeight, wWidth, wHeight - footerHeight, GRAY);

        char stats[64];
        sprintf(stats, "Ln %d, Col %d", activeLine + 1, activeCol);
        DrawTextEx(GetFontDefault(), stats, (Vector2){10, wHeight - footerHeight + 5}, 18, 1, BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
