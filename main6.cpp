/*
 * Saturn Escape: Warp Run
 * A 5-minute automated 3D cinematic experience.
 * Requirements: OpenGL and GLUT.
 * * Timeline:
 * 0-25s:   Intro - Top-down Isometric view of Saturn
 * 25-35s:  SEAMLESS TRANSITION - Fly-by over Saturn
 * 35-90s:  SPACE WARS DOGFIGHT - Dynamic Action Camera & Combat
 * 90-150s: Saturn's Rings entry & Asteroid Field Action
 * 150-210s: Damage & Engine Flicker under Saturn's Shadow
 * 210-230s: ETHEREAL PORTAL FORMATION (Matches new reference)
 * 230-270s: HYPERSPACE TUNNEL
 * 270-290s: NEW GALAXY ARRIVAL
 * 290-300s: Whiteout Fade & "The End" (Strictly 5 Minutes)
 */

#include <GL/freeglut.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>

// --- Configuration & Constants ---
const int REFRESH_MS = 16;
const float TOTAL_TIME = 300.0f;
const int MAX_PARTICLES = 300;
const int ASTEROID_COUNT = 60;
const float PI = 3.14159265359f;

// --- Math Utilities ---
struct Vec3
{
    float x, y, z;
    Vec3 operator+(const Vec3 &v) const { return {x + v.x, y + v.y, z + v.z}; }
    Vec3 operator-(const Vec3 &v) const { return {x - v.x, y - v.y, z - v.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
};

Vec3 lerp(Vec3 a, Vec3 b, float t) { return a + (b - a) * t; }
float lerp(float a, float b, float t) { return a + (b - a) * t; }

enum Emotion
{
    HAPPY,
    DETERMINED,
    WORRIED,
    EXCITED,
    PEACEFUL
};

// --- Global State ---
float currentTime = 0.0f;
float lastTime = 0.0f;
bool isPaused = false;
Emotion currentEmotion = HAPPY;
float heroRoll = 0.0f;
float enemyRoll = 0.0f;

struct Particle
{
    Vec3 pos, vel;
    float life;
    float r, g, b;
    bool active = false;
};

struct Asteroid
{
    Vec3 pos;
    float size;
    float rot;
};

std::vector<Particle> particles(MAX_PARTICLES);
std::vector<Asteroid> asteroids(ASTEROID_COUNT);

// --- UI / Text Rendering ---
void renderUI()
{
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    gluOrtho2D(0, (GLdouble)w, 0, (GLdouble)h);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // 1. Timer & Instructions (Hide during the final sequence)
    if (currentTime < 290.0f)
    {
        int minutes = (int)currentTime / 60;
        int seconds = (int)currentTime % 60;
        std::stringstream ss;
        ss << "MISSION TIME: " << std::setfill('0') << std::setw(2) << minutes << ":"
           << std::setfill('0') << std::setw(2) << seconds;
        if (isPaused)
            ss << " [PAUSED]";
        std::string timeStr = ss.str();

        glColor3f(1.0f, 1.0f, 1.0f);
        glRasterPos2i(30, h - 40);
        for (char c : timeStr)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

        std::string instructions[] = {
            "STARFIGHTER COMMAND:",
            "SPACE   : Pause / Play",
            "L Key   : Skip +10s",
            "J Key   : Back -10s",
            "ESC     : Quit"};

        glColor3f(1.0f, 0.4f, 0.0f);
        int yPos = 120;
        for (const std::string &line : instructions)
        {
            glRasterPos2i(30, yPos);
            for (char c : line)
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
            yPos -= 20;
        }
    }
    // 2. THE END Title Card
    else if (currentTime >= 293.0f)
    {
        std::string endText = "T H E   E N D";
        glColor3f(1.0f, 0.8f, 0.0f); // Yellow text

        int textWidth = 0;
        for (char c : endText)
            textWidth += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, c);
        glRasterPos2i((w - textWidth) / 2, h / 2);
        for (char c : endText)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);

        std::string subText = "Double A's Cinematics";
        glColor3f(0.0f, 0.8f, 1.0f); // Cyan text
        int subWidth = 0;
        for (char c : subText)
            subWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, c);
        glRasterPos2i((w - subWidth) / 2, h / 2 - 30);
        for (char c : subText)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

// --- Effects Engine ---

void drawEtherealPortal(float radius, float zPos)
{
    if (radius <= 0.1f)
        return;

    glPushMatrix();
    glTranslatef(0, 22.0f, zPos);
    glRotatef(currentTime * -80.0f, 0, 0, 1); // Slower, majestic rotation

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for intense glow
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    // 1. Intense Inner Core Ring (White to Bright Cyan)
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= 64; ++i)
    {
        float a = (i / 64.0f) * 2.0f * PI;
        float inner = std::max(0.0f, radius - 1.5f);
        float outer = radius;

        // Pure white inner edge
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glVertex3f(cos(a) * inner, sin(a) * inner, 0);
        // Bright cyan mid-edge
        glColor4f(0.2f, 0.8f, 1.0f, 0.9f);
        glVertex3f(cos(a) * outer, sin(a) * outer, 0);
    }
    glEnd();

    // 2. Swirling Outer Aura (Cyan to Deep Ethereal Blue)
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= 64; ++i)
    {
        float a = (i / 64.0f) * 2.0f * PI;
        float inner = radius;
        // Organic, waving outer edge
        float outer = radius + 4.0f + (float)sin(i * 6.0f + currentTime * 15.0f) * 2.0f;

        glColor4f(0.2f, 0.8f, 1.0f, 0.9f);
        glVertex3f(cos(a) * inner, sin(a) * inner, 0);
        glColor4f(0.0f, 0.2f, 0.8f, 0.0f); // Fades out completely
        glVertex3f(cos(a) * outer, sin(a) * outer, 0);
    }
    glEnd();

    // 3. Radiating Light Rays/Energy Spikes
    glLineWidth(2.5f);
    glBegin(GL_LINES);
    for (int i = 0; i < 150; ++i)
    {
        float a = (i / 150.0f) * 2.0f * PI;
        float rayLength = 5.0f + (float)fmod(sin(i * 123.4f + currentTime * 8.0f) * 20.0f, 15.0f);
        if (rayLength < 0)
            rayLength *= -1;

        float inner = radius - 0.5f;
        float outer = radius + rayLength;

        glColor4f(0.5f, 0.9f, 1.0f, 0.7f); // Bright ray base
        glVertex3f(cos(a) * inner, sin(a) * inner, 0);
        glColor4f(0.0f, 0.1f, 0.6f, 0.0f); // Faded blue tip
        glVertex3f(cos(a) * outer, sin(a) * outer, 0);
    }
    glEnd();

    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void drawHyperspaceTunnel(float camZ)
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    glPushMatrix();
    glTranslatef(0, 22.0f, camZ);

    float tunnelRadius = 25.0f;
    float tunnelLength = 1200.0f;

    // Draw dark blue tunnel walls
    glColor4f(0.0f, 0.05f, 0.2f, 1.0f);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= 40; i++)
    {
        float a = (i / 40.0f) * 2.0f * PI;
        glVertex3f((float)cos(a) * tunnelRadius, (float)sin(a) * tunnelRadius, 0);
        glVertex3f((float)cos(a) * tunnelRadius, (float)sin(a) * tunnelRadius, tunnelLength);
    }
    glEnd();

    // High Speed Light Streaks
    glBegin(GL_QUADS);
    for (int i = 0; i < 400; i++)
    {
        float a = (i / 400.0f) * 2.0f * PI;
        float width = 0.6f;
        float zStart = (float)fmod(i * 123.4f + currentTime * 2500.0f, tunnelLength);
        float len = 150.0f + (i % 100);
        float zEnd = zStart + len;
        float rDist = tunnelRadius - (float)fmod(i * 1.3f, 5.0f);
        float a1 = a - (width / rDist);
        float a2 = a + (width / rDist);

        // Front Head
        glColor4f(0.5f, 0.9f, 1.0f, 0.9f);
        glVertex3f((float)cos(a1) * rDist, (float)sin(a1) * rDist, zStart);
        glVertex3f((float)cos(a2) * rDist, (float)sin(a2) * rDist, zStart);
        // Back Tail
        glColor4f(0.0f, 0.2f, 0.8f, 0.0f);
        glVertex3f((float)cos(a2) * rDist, (float)sin(a2) * rDist, zEnd);
        glVertex3f((float)cos(a1) * rDist, (float)sin(a1) * rDist, zEnd);
    }
    glEnd();

    // Swirling nebula dust inside the tunnel
    glPointSize(6.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 1000; i++)
    {
        float a = (i / 1000.0f) * 2.0f * PI + currentTime * 0.5f;
        float rDist = tunnelRadius - 2.0f - (float)fmod(i * 3.1f, 8.0f);
        float z = (float)fmod(i * 88.0f + currentTime * 1800.0f, tunnelLength);
        float rColor = 0.1f + 0.1f * (float)sin((float)i * 0.1f);
        float gColor = 0.5f + 0.3f * (float)cos((float)i * 0.2f);
        float bColor = 0.9f + 0.1f * (float)sin((float)i * 0.3f);
        glColor4f(rColor, gColor, bColor, 0.5f);
        glVertex3f((float)cos(a) * rDist, (float)sin(a) * rDist, z);
    }
    glEnd();

    glPopMatrix();
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LIGHTING);
}

void drawNewGalaxy(float camZ)
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);

    // Vast colorful starfield
    glPointSize(2.5f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 4000; i++)
    {
        float seed = (float)i;
        float x = (float)sin(seed * 1.23f) * 2000.0f;
        float y = (float)cos(seed * 2.34f) * 2000.0f;
        float z = (float)fmod(seed * 9.87f, 3000.0f) - 1500.0f + camZ;

        // Pink, Purple, Cyan mix
        float r = 0.5f + 0.5f * (float)sin(seed * 0.1f);
        float g = 0.2f + 0.2f * (float)cos(seed * 0.5f);
        float b = 0.7f + 0.3f * (float)sin(seed * 0.3f);

        float twinkle = 0.5f + 0.5f * (float)sin(currentTime * 2.0f + seed);
        glColor4f(r, g, b, twinkle);
        glVertex3f(x, y, z);
    }
    glEnd();

    // Massive distant glowing alien sun
    glPushMatrix();
    glTranslatef(300.0f, 100.0f, camZ + 1200.0f);

    // Glow aura
    glColor4f(1.0f, 0.2f, 0.6f, 0.4f);
    glutSolidSphere(180.0f, 30, 30);

    // Solid core
    glEnable(GL_LIGHTING);
    GLfloat emit[] = {1.0f, 0.3f, 0.6f, 1.0f};
    glMaterialfv(GL_FRONT, GL_EMISSION, emit);
    glColor3f(1.0f, 0.6f, 0.8f);
    glutSolidSphere(150.0f, 40, 40);

    // Reset emission
    GLfloat no_emit[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_EMISSION, no_emit);
    glDisable(GL_LIGHTING);

    glPopMatrix();
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void triggerSparks(Vec3 pos, float r, float g, float b)
{
    int count = 0;
    for (auto &p : particles)
    {
        if (!p.active)
        {
            p.active = true;
            p.pos = pos;
            p.vel = {(float)(rand() % 100 - 50) / 12.0f, (float)(rand() % 100 - 50) / 12.0f, (float)(rand() % 100 - 50) / 12.0f};
            p.life = 1.0f;
            p.r = r;
            p.g = g;
            p.b = b;
            if (++count > 12)
                break;
        }
    }
}

void drawStarfield(float warpFactor, float camZ)
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    for (int layer = 1; layer <= 2; layer++)
    {
        glPointSize((float)layer * 1.2f);
        glBegin(GL_POINTS);
        int count = (layer == 1) ? 2200 : 600;
        for (int i = 0; i < count; i++)
        {
            float seed = (float)i + (float)layer * 1000.0f;
            float x = (float)sin(seed * 0.123f) * 1200.0f;
            float y = (float)cos(seed * 0.456f) * 1200.0f;
            float z = (float)fmod(seed * 987.65f, 2000.0f) - 1400.0f + camZ;
            float twinkle = 0.4f + 0.6f * (float)sin(currentTime * (2.0f + (float)layer) + seed);
            glColor3f(twinkle, twinkle, twinkle);
            glVertex3f(x, y, z);
        }
        glEnd();
    }
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

// --- Ship Geometry ---

void drawFace(Emotion e)
{
    glPushMatrix();
    glTranslatef(0, 0.05f, 1.05f);
    glColor3f(0.01f, 0.01f, 0.01f);
    for (float side : {-0.1f, 0.1f})
    {
        glPushMatrix();
        glTranslatef(side, 0.05f, 0);
        glutSolidSphere(0.035, 10, 10);
        glPopMatrix();
    }
    glDisable(GL_LIGHTING);
    glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP);
    switch (e)
    {
    case HAPPY:
    case PEACEFUL:
        for (float a = -1.0f; a <= 1.01f; a += 0.2f)
            glVertex3f(a * 0.1f, -0.05f + (a * a * 0.03f), 0.01f);
        break;
    case DETERMINED:
        glVertex3f(-0.1f, -0.05f, 0.01f);
        glVertex3f(0.1f, -0.05f, 0.01f);
        break;
    case WORRIED:
        for (float a = -1.0f; a <= 1.01f; a += 0.2f)
            glVertex3f(a * 0.1f, -0.05f - (a * a * 0.03f), 0.01f);
        break;
    case EXCITED:
        for (float a = 0; a < 6.4f; a += 0.5f)
            glVertex3f((float)cos(a) * 0.07f, (float)sin(a) * 0.04f - 0.05f, 0.01f);
        break;
    }
    glEnd();
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void drawHeroShip(Emotion e, float flicker)
{
    float white = 0.95f * flicker;
    float orangeR = 1.0f, orangeG = 0.45f, orangeB = 0.0f;
    glDisable(GL_CULL_FACE);

    glColor3f(white, white, white);

    // Fuselage Top & Bottom
    glBegin(GL_TRIANGLES);
    glVertex3f(0, 0.15f, 1.4f);
    glVertex3f(-0.45f, 0.1f, -1.0f);
    glVertex3f(0.45f, 0.1f, -1.0f);
    glVertex3f(0, -0.1f, 1.4f);
    glVertex3f(-0.45f, -0.1f, -1.0f);
    glVertex3f(0.45f, -0.1f, -1.0f);
    glEnd();

    // Fuselage Sides
    glBegin(GL_QUAD_STRIP);
    glVertex3f(0, 0.15f, 1.3f);
    glVertex3f(0, -0.1f, 1.3f);
    glVertex3f(-0.45f, 0.1f, -0.9f);
    glVertex3f(-0.45f, -0.1f, -0.9f);
    glVertex3f(0.45f, 0.1f, -0.9f);
    glVertex3f(0.45f, -0.1f, -0.9f);
    glVertex3f(0, 0.15f, 1.3f);
    glVertex3f(0, -0.1f, 1.3f);
    glEnd();

    // Dark Thermal Panels (on wings)
    glColor3f(0.15f, 0.15f, 0.15f);
    glBegin(GL_QUADS);
    glVertex3f(-0.5f, 0.06f, -0.1f);
    glVertex3f(-1.3f, 0.06f, -0.8f);
    glVertex3f(-1.3f, 0.06f, -0.9f);
    glVertex3f(-0.5f, 0.06f, -0.7f);
    glVertex3f(0.5f, 0.06f, -0.1f);
    glVertex3f(1.3f, 0.06f, -0.8f);
    glVertex3f(1.3f, 0.06f, -0.9f);
    glVertex3f(0.5f, 0.06f, -0.7f);
    glEnd();

    // Swept-back Wings
    glColor3f(white, white, white);
    glBegin(GL_QUADS);
    glVertex3f(-0.4f, 0.05f, 0.3f);
    glVertex3f(-1.5f, 0.05f, -1.0f);
    glVertex3f(-1.5f, 0.0f, -1.3f);
    glVertex3f(-0.4f, 0.0f, -0.9f);
    glVertex3f(0.4f, 0.05f, 0.3f);
    glVertex3f(1.6f, 0.05f, -1.1f);
    glVertex3f(1.6f, 0.0f, -1.3f);
    glVertex3f(0.4f, 0.0f, -0.9f);
    glEnd();

    // Vertical Tail Fins
    glColor3f(orangeR, orangeG, orangeB);
    glBegin(GL_TRIANGLES);
    glVertex3f(-0.45f, 0.1f, -0.3f);
    glVertex3f(-0.45f, 0.65f, -1.0f);
    glVertex3f(-0.45f, 0.1f, -0.9f);
    glVertex3f(0.45f, 0.1f, -0.3f);
    glVertex3f(0.45f, 0.65f, -1.0f);
    glVertex3f(0.45f, 0.1f, -0.9f);
    glEnd();

    // Glowing Thrusters
    glDisable(GL_LIGHTING);
    float glow = 0.22f + (float)sin(currentTime * 40.0f) * 0.04f;
    for (float sx : {-0.3f, 0.3f})
    {
        glPushMatrix();
        glTranslatef(sx, 0.0f, -1.1f);
        glColor4f(0.1f, 0.8f, 1.0f, 0.6f);
        glutSolidSphere(glow, 12, 12);
        glColor3f(0.9f, 1.0f, 1.0f);
        glutSolidSphere(0.08, 8, 8);
        glPopMatrix();
    }
    glEnable(GL_LIGHTING);
    drawFace(e);
    glEnable(GL_CULL_FACE);
}

void drawEnemyShip()
{
    glPushMatrix();
    glColor3f(0.04f, 0.04f, 0.06f);
    glDisable(GL_CULL_FACE);

    glPushMatrix();
    glScalef(0.45f, 0.45f, 2.8f);
    glutSolidSphere(0.5f, 12, 12);
    glPopMatrix();

    glDisable(GL_LIGHTING);
    glColor3f(0.8f, 0.0f, 1.0f);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(0, 0.1f, 1.2f);
    glVertex3f(-1.4f, 0.2f, 0.4f);
    glVertex3f(-1.7f, -0.1f, -0.9f);
    glVertex3f(0, -0.4f, -0.3f);
    glVertex3f(1.7f, -0.1f, -0.9f);
    glVertex3f(1.4f, 0.2f, 0.4f);
    glEnd();

    glColor3f(0.0f, 0.95f, 1.0f);
    for (float side : {-1.35f, 1.35f})
    {
        glPushMatrix();
        glTranslatef(side, 0.15f, 0.1f);
        glutSolidSphere(0.16f, 10, 10);
        glPopMatrix();
    }
    glEnable(GL_LIGHTING);

    glColor3f(0.1f, 0.1f, 0.15f);
    for (float side : {-1.55f, 1.55f})
    {
        glPushMatrix();
        glTranslatef(side, -0.1f, -0.6f);
        glScalef(0.25f, 0.7f, 0.5f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    glPopMatrix();
    glEnable(GL_CULL_FACE);
}

void drawShip(bool isHero, Emotion e, float flicker = 1.0f)
{
    if (isHero)
        drawHeroShip(e, flicker);
    else
        drawEnemyShip();
}

void drawDogfightLasers(Vec3 hPos, Vec3 ePos)
{
    glDisable(GL_LIGHTING);
    glLineWidth(4.0f);
    if (fmod(currentTime, 1.4f) < 0.2f)
    {
        glColor3f(0.1f, 1.0f, 0.3f);
        float p = fmod(currentTime * 10.0f, 1.0f);
        Vec3 lp = hPos + (ePos - hPos) * p;
        glBegin(GL_LINES);
        glVertex3f(lp.x - 0.7f, lp.y, lp.z);
        glVertex3f(lp.x - 0.7f, lp.y, lp.z + 5);
        glVertex3f(lp.x + 0.7f, lp.y, lp.z);
        glVertex3f(lp.x + 0.7f, lp.y, lp.z + 5);
        glEnd();
        if (p > 0.9f)
            triggerSparks(ePos, 0, 1, 0.3);
    }
    if (fmod(currentTime + 0.7f, 1.1f) < 0.25f)
    {
        glColor3f(1.0, 0.1, 0.1);
        float p = fmod(currentTime * 9.0f, 1.0f);
        Vec3 lp = ePos + (hPos - ePos) * p;
        glBegin(GL_LINES);
        glVertex3f(lp.x - 1.5f, lp.y, lp.z);
        glVertex3f(lp.x - 1.5f, lp.y, lp.z - 6);
        glVertex3f(lp.x + 1.5f, lp.y, lp.z);
        glVertex3f(lp.x + 1.5f, lp.y, lp.z - 6);
        glEnd();
        if (p > 0.9f)
            triggerSparks(hPos, 1, 0.1, 0.1);
    }
    glEnable(GL_LIGHTING);
}

void drawSaturnBody(float radius)
{
    const int stacks = 100, slices = 100;
    GLfloat mat_specular[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat mat_shininess[] = {20.0f};
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    for (int i = 0; i < stacks; ++i)
    {
        float phi1 = PI * (float)i / stacks;
        float phi2 = PI * (float)(i + 1) / stacks;
        float latFactor = (float)i / stacks;
        float r = 0.86f, g = 0.80f, b = 0.68f;
        float noise = (float)sin(phi1 * 20.0f) * 0.02f;
        if (latFactor < 0.15 || latFactor > 0.85)
        {
            r = 0.65f;
            g = 0.65f;
            b = 0.60f;
        }
        else if (latFactor > 0.35 && latFactor < 0.45)
        {
            r = 0.78f;
            g = 0.72f;
            b = 0.60f;
        }
        else if (latFactor > 0.45 && latFactor < 0.48)
        {
            r = 0.60f;
            g = 0.55f;
            b = 0.45f;
        }
        else if (latFactor > 0.48 && latFactor < 0.58)
        {
            r = 0.95f;
            g = 0.92f;
            b = 0.85f;
        }
        glColor3f(r + noise, g + noise, b + noise);
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; ++j)
        {
            float theta = 2.0f * PI * (float)j / slices;
            for (float p : {phi1, phi2})
            {
                float nx = (float)sin(p) * (float)cos(theta), ny = (float)sin(p) * (float)sin(theta), nz = (float)cos(p);
                float x = radius * nx, y = radius * ny, z = radius * nz;
                glNormal3f(nx, ny, nz);
                glVertex3f(x, y, z);
            }
        }
        glEnd();
    }
}

void drawSaturn()
{
    glPushMatrix();
    glRotatef(-110.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(currentTime * 0.5f, 0, 0, 1);
    drawSaturnBody(10.0f);
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glColor4f(0.85f, 0.8f, 0.6f, 0.12f);
    glutSolidSphere(10.2f, 40, 40);
    glDepthMask(GL_TRUE);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    auto drawRing = [](float in, float out, float r, float g, float b, float a)
    {
        glColor4f(r, g, b, a);
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= 360; i += 4)
        {
            float rad = (float)i * PI / 180.0f;
            glVertex3f((float)cos(rad) * in, (float)sin(rad) * in, 0);
            glVertex3f((float)cos(rad) * out, (float)sin(rad) * out, 0);
        }
        glEnd();
    };
    drawRing(12.5f, 14.8f, 0.75f, 0.72f, 0.68f, 0.8f);
    drawRing(14.8f, 15.3f, 0.05f, 0.05f, 0.05f, 0.2f);
    drawRing(15.3f, 18.5f, 0.65f, 0.62f, 0.58f, 0.7f);
    drawRing(18.5f, 21.0f, 0.50f, 0.48f, 0.45f, 0.4f);
    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glPopMatrix();
}

void triggerExplosion(Vec3 pos)
{
    for (auto &p : particles)
    {
        p.active = true;
        p.pos = pos;
        p.vel = {(float)(rand() % 100 - 50) / 20.0f, (float)(rand() % 100 - 50) / 20.0f, (float)(rand() % 100 - 50) / 20.0f};
        p.life = 1.0f;
        p.r = 1.0f;
        p.g = 0.5f;
        p.b = 0.0f;
    }
}

void updatePhysics()
{
    if (isPaused)
        return;
    float dt = currentTime - lastTime;
    for (auto &p : particles)
        if (p.active)
        {
            p.pos = p.pos + p.vel * dt * 9.0f;
            p.life -= dt * 1.2f;
            if (p.life <= 0)
                p.active = false;
        }
}

void display()
{
    // End flashes
    if (currentTime >= 270.0f && currentTime < 273.0f)
    {
        float flash = 1.0f - (currentTime - 270.0f) / 3.0f;
        glClearColor(flash, flash, flash, 1.0f);
    }
    else if (currentTime >= 293.0f)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Solid Black for The End
    }
    else
    {
        glClearColor(0.0f, 0.0f, 0.004f, 1.0f); // Deep Space
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    Vec3 camPos = {0, 0, 0}, lookPos = {0, 0, 0};
    Vec3 hPos = {0, 0, 0}, ePos = {0, 0, 0};
    float warpFactor = 0.0f, flicker = 1.0f;
    bool drawHero = true;

    bool renderPortal = false;
    float pRadius = 0.0f, pZ = 0.0f;
    bool renderTunnel = false;
    bool renderGalaxy = false;

    GLfloat defaultDif[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, defaultDif);

    // --- TIMELINE DISPATCHER ---
    if (currentTime < 25.0f)
    {
        currentEmotion = HAPPY;
        float dist = 50.0f;
        float angle = currentTime * 0.12f;
        camPos = {(float)sin(angle) * dist, 22.0f, (float)cos(angle) * dist};
        lookPos = {0, 0, 0};
        hPos = {0, 0, -400.0f};
        heroRoll = 0;
    }
    else if (currentTime < 35.0f)
    {
        float t = (currentTime - 25.0f) / 10.0f;
        t = t * t * (3.0f - 2.0f * t);
        hPos = lerp({40.0f, 28.0f, -180.0f}, {0.0f, 22.0f, 100.0f}, t);
        camPos = lerp({-10, 35, 90}, {5, 24, 85}, t);
        lookPos = lerp({0, 0, 0}, {0, 22, 180}, t);
    }
    else if (currentTime < 90.0f)
    {
        currentEmotion = DETERMINED;
        float chaseZ = 100.0f + (currentTime - 35.0f) * 7.5f;
        float driftH = (float)sin(currentTime * 0.7f) * 8.0f;
        float driftE = (float)sin(currentTime * 0.7f + 0.8f) * 9.5f;

        hPos = {driftH, 22.0f + (float)cos(currentTime * 1.2f) * 0.8f, chaseZ};
        ePos = {driftE, 22.0f + (float)sin(currentTime * 1.5f) * 2.5f, chaseZ + 18.0f};
        heroRoll = driftH * -5.0f;
        enemyRoll = (driftE - driftH) * -7.0f;

        int cycle = (int)((currentTime - 35.0f) / 5.0f) % 3;
        float shake = (float)(rand() % 10 - 5) * 0.03f;

        switch (cycle)
        {
        case 0:
            camPos = {hPos.x + 0.4f, hPos.y + 0.9f, hPos.z - 0.2f};
            lookPos = {ePos.x, ePos.y, ePos.z};
            break;
        case 1:
            camPos = {ePos.x - 0.3f, ePos.y + 0.6f, ePos.z + 1.5f};
            lookPos = {hPos.x, hPos.y, hPos.z};
            break;
        case 2:
            camPos = {hPos.x + 22.0f, hPos.y + 6.0f, hPos.z + 8.0f};
            lookPos = {hPos.x, hPos.y, chaseZ + 20.0f};
            break;
        }
        camPos.x += shake;
        camPos.y += shake;
    }
    else if (currentTime < 150.0f)
    {
        currentEmotion = DETERMINED;
        float ringZ = 512.5f + (currentTime - 90.0f) * 6.5f;
        float sX = (float)sin(currentTime * 0.9f) * 12.0f;
        hPos = {sX, 22.0f + (float)(sin(currentTime * 2.8f) * 0.4f), ringZ};
        heroRoll = sX * -4.5f;
        camPos = {sX, hPos.y + 4.0f, ringZ - 18.0f};
        lookPos = {sX, hPos.y, ringZ + 20};
    }
    else if (currentTime < 210.0f)
    {
        currentEmotion = WORRIED;
        float darkZ = 902.5f + (currentTime - 150.0f) * 3.0f;
        hPos = {0, 22.0f, darkZ};
        heroRoll = (float)sin(currentTime * 15.0f) * 8.0f;
        flicker = (sin(currentTime * 35.0f) > 0) ? 1.0f : 0.2f;
        camPos = {8.0f, 23.0f, darkZ + 18.0f};
        lookPos = {0, 22.0f, darkZ};
        GLfloat shad[] = {0.03f, 0.03f, 0.05f, 1.0f};
        glLightfv(GL_LIGHT0, GL_DIFFUSE, shad);
    }
    else if (currentTime < 230.0f)
    {
        currentEmotion = DETERMINED;
        float phaseTime = currentTime - 210.0f;
        float portalZ = 1082.5f + 150.0f;

        float portalRadius = std::min(phaseTime * 1.5f, 30.0f);

        float t = phaseTime / 20.0f;
        hPos = {0, 22.0f, lerp(1082.5f, portalZ + 5.0f, t)};
        camPos = {0, 24.0f, hPos.z - 25.0f};
        lookPos = {0, 22.0f, portalZ + 50.0f};

        renderPortal = true;
        pRadius = portalRadius;
        pZ = portalZ;
    }
    else if (currentTime < 270.0f)
    {
        currentEmotion = EXCITED;
        float phaseTime = currentTime - 230.0f; // 0 to 40s
        float warpSpeed = 300.0f;
        float warpZ = 1232.5f + phaseTime * warpSpeed;

        hPos = {0, 22.0f, warpZ + 35.0f};
        heroRoll = (float)sin(currentTime * 8.0f) * 12.0f;

        camPos = {0, 24.0f, warpZ};
        lookPos = {0, 22.0f, hPos.z};
        camPos.x += (float)(rand() % 10 - 5) * 0.08f;
        camPos.y += (float)(rand() % 10 - 5) * 0.08f;

        renderTunnel = true;
    }
    else if (currentTime < 290.0f)
    {
        // NEW GALAXY ARRIVAL
        currentEmotion = PEACEFUL;
        float exitTime = currentTime - 270.0f; // 0 to 20s
        float baseWarpZ = 1232.5f + 40.0f * 300.0f;
        float glideSpeed = 40.0f;
        float currentZ = baseWarpZ + exitTime * glideSpeed;

        hPos = {0, 22.0f, currentZ};
        heroRoll = (float)sin(currentTime * 2.0f) * 3.0f;

        float sweepAngle = exitTime * 0.1f;
        camPos = {(float)sin(sweepAngle) * 30.0f, 25.0f + (float)sin(exitTime * 0.5f) * 5.0f, currentZ - 20.0f + (float)cos(sweepAngle) * 10.0f};
        lookPos = {0, 22.0f, hPos.z + 50.0f};

        renderGalaxy = true;
    }
    else
    {
        drawHero = false;
        camPos = {0, 0, 0};
        lookPos = {0, 0, 1};
    }

    gluLookAt(camPos.x, camPos.y, camPos.z, lookPos.x, lookPos.y, lookPos.z, 0, 1, 0);
    GLfloat light_pos[] = {150.0f, 100.0f, 200.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    // --- DRAW EFFECTS ---
    if (renderPortal)
        drawEtherealPortal(pRadius, pZ);
    if (renderTunnel)
        drawHyperspaceTunnel(camPos.z);
    if (renderGalaxy)
        drawNewGalaxy(camPos.z);

    if (currentTime < 230.0f)
    {
        drawStarfield(warpFactor, camPos.z);
        if (currentTime < 192.0f)
            drawSaturn();

        if (currentTime > 80.0f && currentTime < 170.0f)
        {
            for (int i = 0; i < ASTEROID_COUNT; i++)
            {
                glPushMatrix();
                glTranslatef(asteroids[i].pos.x, asteroids[i].pos.y + 22.0f, asteroids[i].pos.z);
                glRotatef(asteroids[i].rot + currentTime * 50, 0.4, 1, 0.3);
                glColor3f(0.48f, 0.46f, 0.42f);
                glutSolidSphere(asteroids[i].size, 8, 8);
                glPopMatrix();
            }
        }
        if (currentTime > 36.0f && currentTime < 128.0f)
        {
            glPushMatrix();
            glTranslatef(ePos.x, ePos.y, ePos.z);
            glRotatef(enemyRoll, 0, 0, 1);
            drawShip(false, DETERMINED);
            glPopMatrix();
            drawDogfightLasers(hPos, ePos);
        }
    }

    if (drawHero)
    {
        glPushMatrix();
        glTranslatef(hPos.x, hPos.y, hPos.z);
        glRotatef(heroRoll, 0, 0, 1);
        drawShip(true, currentEmotion, flicker);
        glPopMatrix();
    }

    glDisable(GL_LIGHTING);
    glPointSize(3.5f);
    glBegin(GL_POINTS);
    for (auto &p : particles)
        if (p.active)
        {
            glColor4f(p.r, p.g, p.b, p.life);
            glVertex3f(p.pos.x, p.pos.y, p.pos.z);
        }
    glEnd();
    glEnable(GL_LIGHTING);

    renderUI();
    glutSwapBuffers();
}

void timer(int v)
{
    if (!isPaused)
    {
        lastTime = currentTime;
        currentTime += REFRESH_MS / 1000.0f;
        if (currentTime > TOTAL_TIME)
            currentTime = 0;

        float dt = currentTime - lastTime;
        for (auto &p : particles)
        {
            if (p.active)
            {
                p.pos = p.pos + p.vel * dt * 9.0f;
                p.life -= dt * 1.2f;
                if (p.life <= 0)
                    p.active = false;
            }
        }
    }
    glutPostRedisplay();
    glutTimerFunc(REFRESH_MS, timer, 0);
}

void keyboard(unsigned char key, int x, int y)
{
    if (key == 27)
        exit(0);
    if (key == ' ')
        isPaused = !isPaused;
    if (key == 'l' || key == 'L')
        currentTime = std::min(currentTime + 10.0f, TOTAL_TIME - 1.0f);
    if (key == 'j' || key == 'J')
        currentTime = std::max(currentTime - 10.0f, 0.0f);
}

void init()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLfloat dif[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat amb[] = {0.12f, 0.12f, 0.15f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
}

void reshape(int w, int h)
{
    if (h == 0)
        h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / (float)h, 0.1, 4000.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1920, 1080);
    glutCreateWindow("Saturn Escape: Warp Run");
    glutFullScreen();
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(REFRESH_MS, timer, 0);
    glutMainLoop();
    return 0;
}