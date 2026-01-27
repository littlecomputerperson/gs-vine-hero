/*============================================================================================+
 | Game System (GS) Library                                                                   |
 |--------------------------------------------------------------------------------------------|
 | FILES: hero.cpp, hero.h                                                                    |
 |--------------------------------------------------------------------------------------------|
 | CLASS: Hero                                                                                |
 |--------------------------------------------------------------------------------------------|
 | ABOUT: Class to create a hero                                                              |
 |--------------------------------------------------------------------------------------------|
 |                                                                                    04/2019 |
 +============================================================================================*/


//==============================================================================================
// Include Game System (GS) header files.
// ---------------------------------------------------------------------------------------------
#include "hero.h"
//==============================================================================================


////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor/Destructor Methods //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

//==============================================================================================
// Hero::Hero():
// ---------------------------------------------------------------------------------------------
// Purpose: The constructor, initializes class data when class object is created.
// ---------------------------------------------------------------------------------------------
// Returns: Nothing.
//==============================================================================================

Hero::Hero()
{
    m_isCreated = false;

    m_posX = 0;
    m_posY = 0;

    m_selectedHero = SELECT_HERO_JOHNNY;

    m_heroFrame = 0;

    m_state = HERO_FRAME_STANDING;

    m_velocityX = HERO_JUMP_VELOCITY_X;
    m_velocityY = HERO_JUMP_VELOCITY_Y;
}

////////////////////////////////////////////////////////////////////////////////////////////////

//==============================================================================================
// Hero::~Hero():
// ---------------------------------------------------------------------------------------------
// Purpose: The de-structor, de-initializes class data when class object is destroyed.
// ---------------------------------------------------------------------------------------------
// Returns: Nothing.
//==============================================================================================

Hero::~Hero()
{
    this->Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////////////

//==============================================================================================
// Hero::Create():
// ---------------------------------------------------------------------------------------------
// Purpose:
// ---------------------------------------------------------------------------------------------
// Returns: Nothing.
//==============================================================================================

bool Hero::Create(int selectedHero, int startX, int startY, bool filtered)
{
    bool result;

    // Has the hero already been created?
    if( m_isCreated )
    {
        this->Destroy();
    }

    // Set hero details
    m_posX = ( startX > -1 ? startX : m_posX );
    m_posY = ( startY > -1 ? startY : m_posY );
    m_selectedHero = selectedHero;

    // Depending on the hero selected
    if( selectedHero == SELECT_HERO_COURAGE )
    {
        // Create a sprite with 8 frames, 4 frames per line and dimensions of 128x128
        result = m_heroSprite.Create("data/hero_02.tga", 8, 4, 128, 128, filtered, filtered);
    }
    else
    {
        // Create a sprite with 8 frames, 4 frames per line and dimensions of 128x128
        result = m_heroSprite.Create("data/hero_01.tga", 8, 4, 128, 128, filtered, filtered);
        m_selectedHero = SELECT_HERO_JOHNNY;
    }

    // Update values that could have been lost when re-creting the sprite
    m_heroSprite.SetDestXY(m_posX, m_posY);
    m_heroSprite.SetFrame(m_heroFrame);

    // Remember that the hero was created
    m_isCreated = result;

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////

//==============================================================================================
// Hero::Destroy():
// ---------------------------------------------------------------------------------------------
// Purpose: The deatroys the hero
// ---------------------------------------------------------------------------------------------
// Returns: Nothing.
//==============================================================================================

void Hero::Destroy()
{
    m_isCreated = false;

    m_posX = 0;
    m_posY = 0;

    m_selectedHero = SELECT_HERO_JOHNNY;

    m_heroFrame = 0;

    m_state = HERO_FRAME_STANDING;

    m_velocityX = HERO_JUMP_VELOCITY_X;
    m_velocityY = HERO_JUMP_VELOCITY_Y;

    m_heroSprite.Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////////////

//==============================================================================================
// Hero::SetState():
// ---------------------------------------------------------------------------------------------
// Purpose: Set the hero state
// ---------------------------------------------------------------------------------------------
// Returns: Nothing
//==============================================================================================

void Hero::SetState( int stateNum )
{
    // Can't change to the current state
    if( stateNum == m_state )
    {
        return;
    }

    // Depending on the state selected
    if( stateNum == HERO_STATE_STANDING )
    {
        m_heroSprite.SetFrame(HERO_FRAME_STANDING);
        m_heroFrame = HERO_FRAME_STANDING;

        m_heroSprite.SetRotationPointXY(-1, -1);
        m_heroSprite.SetRotateZ(0);

        m_heroSprite.SetScaleXY(1.0f, 1.0f);
    }
    else if( stateNum == HERO_STATE_SWINGING )
    {
        m_heroSprite.SetFrame(HERO_FRAME_SWINGING);
        m_heroFrame = HERO_FRAME_SWINGING;

        // Set rotation point depending on the hero selected
        if( m_selectedHero == SELECT_HERO_COURAGE )
        {
            m_heroSprite.SetRotationPointXY(14, m_heroSprite.GetHeight() - 19);
        }
        else
        {
            m_heroSprite.SetRotationPointXY(10, m_heroSprite.GetHeight() - 23);
        }
        m_heroSprite.SetRotateZ(0);

        m_heroSprite.SetScaleXY(1.0f, 1.0f);
    }
    else if( stateNum == HERO_STATE_JUMPING )
    {
        m_velocityX = HERO_JUMP_VELOCITY_X;
        m_velocityY = HERO_JUMP_VELOCITY_Y;

        m_heroSprite.SetFrame(HERO_FRAME_JUMPING);
        m_heroFrame = HERO_FRAME_JUMPING;

        m_heroSprite.SetRotationPointXY(-1, -1);
        m_heroSprite.SetRotateZ(0);

        m_heroSprite.SetScaleXY(1.0f, 1.0f);
    }
    else if( stateNum == HERO_STATE_FALLING )
    {
        m_velocityX = HERO_FALL_VELOCITY_X;
        m_velocityY = HERO_FALL_VELOCITY_Y;

        m_heroSprite.SetFrame(HERO_FRAME_FALLING);
        m_heroFrame = HERO_FRAME_FALLING;

        m_heroSprite.SetRotationPointXY(-1, -1);
        m_heroSprite.SetRotateZ(0);

        m_heroSprite.SetScaleXY(1.0f, 1.0f);
    }
    else
    {
        return;
    }

    // Remember the hero state
    m_state = stateNum;
}

////////////////////////////////////////////////////////////////////////////////////////////////

//==============================================================================================
// Hero::GetDestRect():
// ---------------------------------------------------------------------------------------------
// Purpose: Get the screen coordinates of the collision area of the hero
// ---------------------------------------------------------------------------------------------
// Returns: Nothing
//==============================================================================================

void Hero::GetCollideRect(RECT* pSourceRect)
{
    m_heroSprite.GetDestRect(pSourceRect);

    // Make adjustments depending on the hero state
    if( m_state == HERO_STATE_JUMPING )
    {
        // Depending on the selected hero
        if( m_selectedHero == SELECT_HERO_JOHNNY )
        {
            pSourceRect->top = pSourceRect->top + 10;
            pSourceRect->left = pSourceRect->left - 10;
            pSourceRect->bottom = pSourceRect->top - 30;
            pSourceRect->right = pSourceRect->left + 30;
        }
        else if( m_selectedHero == SELECT_HERO_COURAGE )
        {
            pSourceRect->top = pSourceRect->top - 10;
            pSourceRect->left = pSourceRect->left + 5;
            pSourceRect->bottom = pSourceRect->top - 30;
            pSourceRect->right = pSourceRect->left + 30;
        }
    }
    else if( m_state == HERO_STATE_SWINGING )
    {
        pSourceRect->top = pSourceRect->top - 60;
        pSourceRect->left = pSourceRect->left - 5;
        pSourceRect->bottom = pSourceRect->top - 20;
        pSourceRect->right = pSourceRect->left + 20;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////

//==============================================================================================
// Hero::UpdateMovement():
// ---------------------------------------------------------------------------------------------
// Purpose: Update the hero's position when jumping
// ---------------------------------------------------------------------------------------------
// Returns: Nothing
//==============================================================================================

void Hero::UpdateMovement(float rotateZ)
{
    float acceleration = HERO_ACCELERATION;

    // Get current position of hero
    m_posX = m_heroSprite.GetDestX();
    m_posY = m_heroSprite.GetDestY();

    // Is the hero swinging?
    if( m_state == HERO_STATE_SWINGING )
    {
        // Do rotation
        m_heroSprite.SetRotateZ( rotateZ );
    }

    // Is the hero jumping?
    if( m_state == HERO_STATE_JUMPING )
    {
        // Has the sprite left the screen?
        if( m_posY < (0 -  m_heroSprite.GetFrameHeight()) )
        {
            // Stop adjusting the postion
            return;
        }

        // Adjust the vertical velocity of the hero based on the acceleration
        m_velocityY = m_velocityY - acceleration;

        // Calculate the new coordinates
        m_posX = m_posX - m_velocityX;
        m_posY = m_posY + m_velocityY;

        // Do rotation
        m_heroSprite.AddRotateZ( HERO_JUMP_ROTATE_Z );
    }

    // Is the hero falling?
    if( m_state == HERO_STATE_FALLING )
    {
        // Has the sprite left the screen?
        if( m_posY < (0 -  m_heroSprite.GetFrameHeight()) )
        {
            // Stop adjusting the postion
            return;
        }

        // Adjust the vertical velocity of the hero based on the acceleration
        m_velocityY = m_velocityY - acceleration;

        // Calculate the new coordinates
        m_posX = m_posX - m_velocityX;
        m_posY = m_posY + m_velocityY;

        // Do rotation
        m_heroSprite.AddRotateZ( HERO_FALL_ROTATE_Z );

        // Do scaling
        m_heroSprite.AddScaleX(HERO_FALL_SCALE);
        m_heroSprite.AddScaleY(HERO_FALL_SCALE);
    }

    // Update the hero position
    m_heroSprite.SetDestXY(m_posX, m_posY);

    return;
}

////////////////////////////////////////////////////////////////////////////////////////////////

//==============================================================================================
// Hero::Render():
// ---------------------------------------------------------------------------------------------
// Purpose: Draws the hero
// ---------------------------------------------------------------------------------------------
// Returns: True if rendering was successful, false if not
//==============================================================================================

bool Hero::Render(float fAlpha)
{
    // Set the alpha value to determine the transparency
    m_heroSprite.SetModulateColor(-1.0f, -1.0f, -1.0f, fAlpha);

    // Render the hero
    m_heroSprite.Render();

    return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////
