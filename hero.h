/*============================================================================================+
 | Game System (GS) Library                                                                   |
 |--------------------------------------------------------------------------------------------|
 | FILES: hero.cpp, hero.h                                                                    |
 |--------------------------------------------------------------------------------------------|
 | CLASS: Hero                                                                                |
 |--------------------------------------------------------------------------------------------|
 | ABOUT: Class to create and render a swinging hero                                          |
 |--------------------------------------------------------------------------------------------|
 |                                                                                    04/2019 |
 +============================================================================================*/


#ifndef HERO_H
#define HERO_H


//================================================================================================
// Include Game System (GS) header files.
// -----------------------------------------------------------------------------------------------
#include "gs_list.h"
#include "gs_ogl_texture.h"
#include "gs_ogl_sprite_ex.h"

//================================================================================================


//================================================================================================
// Include standard C library header files.
// -----------------------------------------------------------------------------------------------
#include <time.h>
//================================================================================================


//==============================================================================================
// Hero defines.
// ---------------------------------------------------------------------------------------------
#define SELECT_HERO_JOHNNY  0
#define SELECT_HERO_COURAGE 1
// ---------------------------------------------------------------------------------------------
#define HERO_FRAME_STANDING 0
#define HERO_FRAME_SWINGING 1
#define HERO_FRAME_JUMPING  2
#define HERO_FRAME_FALLING  3
// ---------------------------------------------------------------------------------------------
#define HERO_STATE_STANDING 0
#define HERO_STATE_SWINGING 1
#define HERO_STATE_JUMPING  2
#define HERO_STATE_FALLING  3
// ---------------------------------------------------------------------------------------------
#define HERO_ACCELERATION     1.5f
#define HERO_JUMP_VELOCITY_X  11.0f
#define HERO_JUMP_VELOCITY_Y  21.0f
#define HERO_JUMP_ROTATE_Z    0.0f
#define HERO_FALL_VELOCITY_X  5.0f
#define HERO_FALL_VELOCITY_Y  20.0f
#define HERO_FALL_ROTATE_Z    5.0f
#define HERO_FALL_SCALE       0.02f
// ---------------------------------------------------------------------------------------------


////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definition. ///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


class Hero
{

private:

    GS_OGLSpriteEx m_heroSprite;

    bool m_isCreated;

    int m_selectedHero; // The index of the selected hero

    int m_posX; // The X coordinate in pixels of the hero
    int m_posY; // The Y coordinate in pixels of the hero

    int m_heroFrame; // The currently selected hero frame (see defines above for available frames)

    int m_state;  // The state of the hero (see defines above for available states)

    float m_velocityX;
    float m_velocityY;

    // Set the animation fram for the hero sprite
    inline void SetFrame( int frameNum )
    {
        m_heroSprite.SetFrame(frameNum);
    }

protected:

    // No protected members.

public:

    Hero();
    ~Hero();

    bool Create
    (
        int selectedHero,
        int startX = -1,
        int startY = -1,
        bool filtered = false
    );

    void Destroy();

    // Set the hero state
    void SetState( int stateNum );

    // Get the selected hero
    inline int GetSelectedHero()
    {
        return m_selectedHero;
    }

    // Get the hero state
    inline int GetState()
    {
        return m_state;
    }

    // Set the position for the hero sprite
    inline void SetPosition( int x, int y )
    {
        m_heroSprite.SetDestXY(x, y);
    }

    inline int GetDestX()
    {
        return m_heroSprite.GetDestX();
    }

    inline int GetDestY()
    {
        return m_heroSprite.GetDestY();
    }

    inline int GetWidth()
    {
        return m_heroSprite.GetFrameWidth();
    }

    inline int GetHeight()
    {
        return m_heroSprite.GetFrameHeight();
    }

    void GetCollideRect(RECT* pSourceRect);

    // Update the hero's position depending on the state
    void UpdateMovement(float rotateZ = 0.0f);

    bool Render(float fAlpha = 1.0f); // Render the hero
};

////////////////////////////////////////////////////////////////////////////////////////////////

#endif
