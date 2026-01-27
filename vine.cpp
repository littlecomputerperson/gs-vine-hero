/*============================================================================================+
 | Game System (GS) Library                                                                   |
 |--------------------------------------------------------------------------------------------|
 | FILES: vine.cpp, vine.h                                                                    |
 |--------------------------------------------------------------------------------------------|
 | CLASS: Vine                                                                                |
 |--------------------------------------------------------------------------------------------|
 | ABOUT: Class to create and render a swinging vine                                          |
 |--------------------------------------------------------------------------------------------|
 |                                                                                    04/2019 |
 +============================================================================================*/


//==============================================================================================
// Include Game System (GS) header files.
// ---------------------------------------------------------------------------------------------
#include "vine.h"
//==============================================================================================


////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor/Destructor Methods //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

//==============================================================================================
// Vine::Vine():
// ---------------------------------------------------------------------------------------------
// Purpose: The constructor, initializes class data when class object is created.
// ---------------------------------------------------------------------------------------------
// Returns: Nothing.
//==============================================================================================

Vine::Vine()
{
    m_isCreated = false;

    m_startX = 0;
    m_startY = 0;

    m_swingMaxAmplitude = 200;
    m_swingMinAmplitude = 180;
    m_swingWidth = 100;
    m_swingSpeed = 50;

    m_swingControlX1 = 0;
    m_swingControlY1 = 0;
    m_swingControlX2 = 0;
    m_swingControlY2 = 0;

    m_swingStartX = 0;
    m_swingStartY = 0;
    m_swingEndX = 0;
    m_swingEndY = 0;

    m_swingMoveAmt = 0.0f;
    m_angle = 270.0f;

    m_tightness = SPRING_TIGHTNESS;
    m_damping = SPRING_DAMPING;
    m_velocity = 0.0f;

    m_isSwingingLeft = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////

//==============================================================================================
// Vine::~Vine():
// ---------------------------------------------------------------------------------------------
// Purpose: The de-structor, de-initializes class data when class object is destroyed.
// ---------------------------------------------------------------------------------------------
// Returns: Nothing.
//==============================================================================================

Vine::~Vine()
{
    this->Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////////////

//==============================================================================================
// Vine::Create():
// ---------------------------------------------------------------------------------------------
// Purpose:
// ---------------------------------------------------------------------------------------------
// Returns: Nothing.
//==============================================================================================

bool Vine::Create(int numSprites, int startX, int startY, float swingSpeed, int swingMaxAmplitude, int swingMinAmplitude, int swingWidth)
{
    int spriteHeight = 0;

    // The vine must have at least 3 sprites
    if( numSprites < 3 )
    {
        return false;
    }

    // Has the vine been created?
    if( m_isCreated )
    {
        this->Destroy();
    }

    // Create the texture for the vine sprites
    m_vineSpriteTexture.Create("data/vine.tga", GS_MIPMAP, GL_NEAREST, GL_NEAREST);

    // For every sprite in the vine
    for( int i = 0; i < numSprites; i++ )
    {
        // Create the sprite and add it to the list
        GS_OGLSprite* newSprite = new GS_OGLSprite;

        if( !newSprite->Create(m_vineSpriteTexture.GetID(), 16, 16) )
        {
            return false;
        }

        if( spriteHeight == 0)
        {
            spriteHeight = newSprite->GetHeight();
        }

        m_vineSprites.AddItem(newSprite);
    }

    // Set vine details
    m_startX = startX;
    m_startY = startY;
    m_swingSpeed = swingSpeed;
    m_angle = (rand()%90) * (swingSpeed / MAX_SWING_SPEED) / 2;
    m_angle = (rand()%2 == 1) ? 270.0f + m_angle : 270.0f - m_angle;

    // Set default swing max amplitude
    m_swingMaxAmplitude = numSprites * (spriteHeight / VINE_ELEMENT_PROX);

    // Overwrite default max amplitude if value is provided
    if( swingMaxAmplitude > 0 )
    {
        m_swingMaxAmplitude = swingMaxAmplitude;
    }

    // Set default swing min amplitude
    m_swingMinAmplitude = m_swingMaxAmplitude - (m_swingMaxAmplitude * m_swingSpeed / MAX_SWING_SPEED);

    // Overwrite default min amplitude if value is provided
    if( swingMinAmplitude > 0 )
    {
        m_swingMinAmplitude = swingMinAmplitude;
    }

    // Set default swing length
    m_swingWidth = m_swingMaxAmplitude * 2 + (m_swingMinAmplitude * 2 * m_swingSpeed / MAX_SWING_SPEED);

    // Overwrite default swing length if value is provided
    if( swingWidth > 0 )
    {
        m_swingWidth = swingWidth;
    }

    // Set the starting position if each sprite in the vine
    int xPos = m_startX;
    int yPos = m_startY;

    for (int i = 0; i < m_vineSprites.GetNumItems(); i++)
    {
        // Get the sprite in the list
        GS_OGLSprite* vineSprite = m_vineSprites.GetItem(i);

        // Set the spirte starting coordinates
        vineSprite->SetDestXY(xPos, yPos);

        // Adjust the vertical position
        yPos = yPos - (vineSprite->GetHeight() / VINE_ELEMENT_PROX);
    }

    // Calculate swing details
    m_swingControlX1 = m_startX - ((m_swingWidth / 2) - (m_swingWidth / 4));
    m_swingControlY1 = m_startY - (m_swingMaxAmplitude + (m_swingMinAmplitude / 4));
    m_swingControlX2 = m_startX + ((m_swingWidth / 2) - (m_swingWidth / 4));
    m_swingControlY2 = m_startY - (m_swingMaxAmplitude + (m_swingMinAmplitude / 4));

    m_swingStartX = m_swingControlX1;
    m_swingStartY = m_startY - m_swingMinAmplitude;
    m_swingEndX = m_swingControlX2;
    m_swingEndY = m_startY - m_swingMinAmplitude;

    m_isCreated = true;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////

//==============================================================================================
// Vine::Destroy():
// ---------------------------------------------------------------------------------------------
// Purpose: The deatroys the vine
// ---------------------------------------------------------------------------------------------
// Returns: Nothing.
//==============================================================================================

void Vine::Destroy()
{
    m_isCreated = false;

    m_startX = 0;
    m_startY = 0;

    m_swingMaxAmplitude = 200;
    m_swingMinAmplitude = 180;
    m_swingWidth = 100;
    m_swingSpeed = 50;

    m_swingControlX1 = 0;
    m_swingControlY1 = 0;
    m_swingControlX2 = 0;
    m_swingControlY2 = 0;

    m_swingStartX = 0;
    m_swingStartY = 0;
    m_swingEndX = 0;
    m_swingEndY = 0;

    m_swingMoveAmt = 0.0f;
    m_angle = 270.0f;

    m_tightness = SPRING_TIGHTNESS;
    m_damping = SPRING_DAMPING;
    m_velocity = 0.0f;

    m_isSwingingLeft = false;

    // Destroy the vine texture
    m_vineSpriteTexture.Destroy();

    // Destroy the vine
    for (int i = 0; i < m_vineSprites.GetNumItems(); i++)
    {
        delete m_vineSprites.GetItem(i);
    }
    m_vineSprites.RemoveAllItems();
}

////////////////////////////////////////////////////////////////////////////////////////////////

//==============================================================================================
// Vine::AddDestX():
// ---------------------------------------------------------------------------------------------
// Purpose: Update the horizonatal position of the vine
// ---------------------------------------------------------------------------------------------
// Returns: Nothing
//==============================================================================================

void Vine::AddDestX(int x)
{
    // Update starting postion
    m_startX = m_startX + x;

    // Update swing details
    m_swingControlX1 = m_startX - ((m_swingWidth / 2) - (m_swingWidth / 4));
    m_swingControlX2 = m_startX + ((m_swingWidth / 2) - (m_swingWidth / 4));
    m_swingStartX = m_swingControlX1;
    m_swingEndX = m_swingControlX2;

    // Update the postions of the individual vine elements
    for (int i = 0; i < m_vineSprites.GetNumItems(); i++)
    {
        // Get the sprite in the list
        GS_OGLSprite* vineSprite = m_vineSprites.GetItem(i);

        // Set the spirte starting coordinates
        vineSprite->AddDestX(x);

    }
}

////////////////////////////////////////////////////////////////////////////////////////////////

//==============================================================================================
// Vine::UpdateBezierMovement():
// ---------------------------------------------------------------------------------------------
// Purpose: Update the coordinates of the vine along a bezier curve
// ---------------------------------------------------------------------------------------------
// Returns: Nothing
//==============================================================================================

void Vine::UpdateBezierMovement()
{
    // static float swingMoveFrac = m_swingMoveFrac;
    int lastSpriteIndex = m_vineSprites.GetNumItems() - 1;
    float moveX, moveY, acceleration;
    GS_OGLSprite* lastVineSprite;

    // Get the last vine sprite
    lastVineSprite = m_vineSprites.GetItem( lastSpriteIndex );

    // Calculate the acceleration based on the relative position of the last element of the vine
    // to the amplitude of the swing
    acceleration = ((float)(m_swingEndY - lastVineSprite->GetDestY()) /
        (float)(m_swingMaxAmplitude - m_swingMinAmplitude));

    // Make sure the acceleration is withing accepatble limits
    if( acceleration < 0.1f )
    {
        acceleration = 0.1f;
    }
    else if( acceleration > 1.0f )
    {
        acceleration = 1.0f;
    }

    // Should we start moving in the other direction?
    if( m_swingMoveAmt > 1.0f )
    {
        m_isSwingingLeft = !m_isSwingingLeft;
        m_swingMoveFrac = m_swingMoveFrac * -1.0f;
        m_swingMoveAmt = 1.0f;

    }
    else if( m_swingMoveAmt < 0.0f )
    {
        m_isSwingingLeft = !m_isSwingingLeft;
        m_swingMoveFrac = m_swingMoveFrac * -1.0f;
        m_swingMoveAmt = 0.0f;
    }

    // Calculate the total movement amount based on the swing speed and acceleration
    m_swingMoveAmt = m_swingMoveAmt + (float)((m_swingMoveFrac / 100.0f) * m_swingSpeed * acceleration);

    // Adjust the position of the last element of the vine along a bezier curve
    moveX =
        pow(m_swingMoveAmt, 3) * (m_swingEndX + 3 * (m_swingControlX1 - m_swingControlX2) - m_swingStartX) +
        3 * pow(m_swingMoveAmt, 2) * (m_swingStartX - 2 * m_swingControlX1 + m_swingControlX2) +
        3 * m_swingMoveAmt * (m_swingControlX1 - m_swingStartX) + m_swingStartX;

    moveY =
        pow(m_swingMoveAmt, 3) * (m_swingEndY + 3 * (m_swingControlY1 - m_swingControlY2) - m_swingStartY) +
        3 * pow(m_swingMoveAmt, 2) * (m_swingStartY - 2 * m_swingControlY1 + m_swingControlY2) +
        3 * m_swingMoveAmt * (m_swingControlY1 - m_swingStartY) + m_swingStartY;

    lastVineSprite->SetDestXY((GLint)moveX, (GLint)moveY);

    // Calculate velocity based on swing speed
    m_velocity = m_swingSpeed / MAX_SWING_SPEED;

    // Add the spring elements between the first and last element of the vine and adjust all the
    // elements in-between
    int springInterval = VINE_SPRING_INTERVAL;
    int prevSpringIndex = 0;
    int thisSpringIndex = 0;
    int nextSpringIndex = 0;

    float addX = 0;
    float addY = 0;

    GS_OGLSprite* prevSprite;
    GS_OGLSprite* thisSprite;
    GS_OGLSprite* nextSprite;
    GS_OGLSprite* tempSprite;

    for (int i = 0; i < m_vineSprites.GetNumItems(); i += springInterval)
    {
        // Get the indexes of a set of elements used as springs
        prevSpringIndex = i;
        thisSpringIndex = i + springInterval;
        nextSpringIndex = thisSpringIndex + springInterval;

        // Is the lat element in the set still in bounds?
        if( nextSpringIndex >= m_vineSprites.GetNumItems() )
        {
            // Is the current element less than the last element avialable?
            if( thisSpringIndex < m_vineSprites.GetNumItems() - 1 )
            {
                // Set the last element in the set to the last element available
                nextSpringIndex = m_vineSprites.GetNumItems() - 1;
            }
            else
            {
                break;
            }
        }

        // Get the set of elements for the springs
        prevSprite = m_vineSprites.GetItem(prevSpringIndex);
        thisSprite = m_vineSprites.GetItem(thisSpringIndex);
        nextSprite = m_vineSprites.GetItem(nextSpringIndex);

        // Calculate the amount the movement of the current sprite needs to be adjusted based on the
        // spring physics between the next and previous springs
        addX = (float(nextSprite->GetDestX() - prevSprite->GetDestX()) * m_tightness) - float(m_velocity * m_damping);
        addY = (float(nextSprite->GetDestY() - prevSprite->GetDestY()) * m_tightness) - float(m_velocity * m_damping);

        // Set the adjusted position of the current spring
        thisSprite->SetDestX( prevSprite->GetDestX() + addX );
        thisSprite->SetDestY( prevSprite->GetDestY() + addY );

        // Adjust the elements in-between the first spring and the current one
        for( int j = prevSpringIndex + 1; j < thisSpringIndex; j++ )
        {
            addX = float((thisSprite->GetDestX() - prevSprite->GetDestX()) / float(springInterval)) * (thisSpringIndex-j);
            addY = float((thisSprite->GetDestY() - prevSprite->GetDestY()) / float(springInterval)) * (thisSpringIndex-j);

            tempSprite = m_vineSprites.GetItem(j);
            tempSprite->SetDestX( prevSprite->GetDestX() + addX );
            tempSprite->SetDestY( prevSprite->GetDestY() + addY );
        }
    }

    // Get the index after the last spring that was drawn
    thisSpringIndex = thisSpringIndex - springInterval + 1;

    // Are there more elements to be drawn?
    if( thisSpringIndex < m_vineSprites.GetNumItems() )
    {
        // Get the last spring element
        thisSprite = m_vineSprites.GetItem(thisSpringIndex - 1);

        // Adjust all elements in the vine after the last spring element
        for( int j = thisSpringIndex; j < m_vineSprites.GetNumItems(); j++ )
        {
            tempSprite = m_vineSprites.GetItem(j);
            tempSprite->SetDestX( thisSprite->GetDestX() + (addX * (m_vineSprites.GetNumItems() - j)) );
            tempSprite->SetDestY( thisSprite->GetDestY() + (addY * (m_vineSprites.GetNumItems() - j)) );
        }
    }

    return;
}

////////////////////////////////////////////////////////////////////////////////////////////////

//==============================================================================================
// Vine::UpdateCircleMovement():
// ---------------------------------------------------------------------------------------------
// Purpose: Update the coordinates of the vine along the circumfrance of a circle
// ---------------------------------------------------------------------------------------------
// Returns: Nothing
//==============================================================================================

void Vine::UpdateCircleMovement()
{
    int lastSpriteIndex = m_vineSprites.GetNumItems() - 1;
    float moveX, moveY, acceleration;
    float swingSpeedMod = 20.0f;
    GS_OGLSprite* vineSprite;

    // Get the last vine sprite
    vineSprite = m_vineSprites.GetItem( lastSpriteIndex );

    // The swing speed modifier is used to bring the swing speed inline with the bezier function
    // It should never adjust the swing speed to more than 100.
    if( m_swingSpeed + swingSpeedMod > MAX_SWING_SPEED )
    {
        swingSpeedMod = MAX_SWING_SPEED - m_swingSpeed;
    }

    // Calculate accelecration based on the distance of angle from the resting position (i.e. 270)
    if( m_angle > 270.0f )
    {
        float a = (m_angle - 270.0f);
        float b = 90.0f * ((m_swingSpeed + swingSpeedMod) * 0.01f);
        acceleration = (1.0f - (a / b)) / 1.0f;
    }
    else
    {
        float a = (270.0f - m_angle);
        float b = 90.0f * ((m_swingSpeed + swingSpeedMod) * 0.01f);
        acceleration = (1.0f - ( a / b)) / 1.0f;
    }

    // Has the vine stopped swinging?
    if( acceleration <= 0.09f )
    {
        // Start swinging the other way
        m_isSwingingLeft = !m_isSwingingLeft;
        m_swingMoveFrac = m_swingMoveFrac * -1.0f;
        acceleration = 0.1f;
    }

    // Adjust the angle for the swing
    m_angle += (m_swingMoveFrac * m_swingSpeed * acceleration);

    // Set the new coordinates for the bottom vine element
    moveX = m_startX + m_swingMaxAmplitude * cos(m_angle * M_PI / 180);
    moveY = m_startY + m_swingMaxAmplitude * sin(m_angle * M_PI / 180);

    // Update the coordinates of the bottom vine element
    vineSprite->SetDestXY((GLint)moveX, (GLint)moveY);

    // Adjust the position of the other vine elements
    for (int i = 1; i < m_vineSprites.GetNumItems() - 1; i++)
    {
        float swingAmplitude = 0.0f;

        GS_OGLSprite* thisSprite = m_vineSprites.GetItem(i);

        swingAmplitude = (float)(m_swingMaxAmplitude * (float)i / (float)(m_vineSprites.GetNumItems() - 1));

        // Set the new coordinates for the bottom vine element
        moveX = m_startX + (int)(swingAmplitude * cos(m_angle * M_PI / 180));
        moveY = m_startY + (int)(swingAmplitude * sin(m_angle * M_PI / 180));

        // Update the coordinates of the bottom vine element
        thisSprite->SetDestXY((GLint)moveX, (GLint)moveY);
    }

    return;
}

////////////////////////////////////////////////////////////////////////////////////////////////

//==============================================================================================
// Vine::UpdateSwing():
// ---------------------------------------------------------------------------------------------
// Purpose: Calculates the vine position
// ---------------------------------------------------------------------------------------------
// Returns: Nothing
//==============================================================================================

void Vine::UpdateSwing( bool useBezier )
{
    // Should a bezier curve be used?
    if( useBezier )
    {
        // Update the vine movement using a bezier curve
        UpdateBezierMovement();
    }
    else
    {
        // Update the vine movement using a circle
        UpdateCircleMovement();
    }

    return;
}

////////////////////////////////////////////////////////////////////////////////////////////////

//==============================================================================================
// Vine::Render():
// ---------------------------------------------------------------------------------------------
// Purpose: Draws the vine
// ---------------------------------------------------------------------------------------------
// Returns: True if rendering was successful, false if not
//==============================================================================================

bool Vine::Render(float fAlpha)
{
    // For every vine sprite
    for (int i = 0; i < m_vineSprites.GetNumItems(); i++)
    {
        // Get the sprite in the list
        GS_OGLSprite* vineSprite = m_vineSprites.GetItem(i);

        // Set the alpha value to determine the transparency
        vineSprite->SetModulateColor(-1.0f, -1.0f, -1.0f, fAlpha);

        // Render the sprite
        vineSprite->Render();
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////

//==============================================================================================
// Vine::GetCollideElement():
// ---------------------------------------------------------------------------------------------
// Purpose: Draws the vine
// ---------------------------------------------------------------------------------------------
// Returns: True if rendering was successful, false if not
//==============================================================================================

int Vine::GetCollideElement(RECT rcSource)
{
    RECT rcDest;

    // For every vine sprite
    for (int i = m_vineSprites.GetNumItems() -1; i >= 0; i--)
    {
        // Get the sprite in the list
        GS_OGLSprite* vineSprite = m_vineSprites.GetItem(i);

        // Get the destination rectangle
        vineSprite->GetDestRect(&rcDest);

        // Return the element number
        if( m_gsCollide.IsRectOnRect(rcSource, rcDest, 100) )
        {
            return i;
            break;
        }
    }

    return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////
