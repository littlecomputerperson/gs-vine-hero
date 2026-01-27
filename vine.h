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


#ifndef VINE_H
#define VINE_H


//================================================================================================
// Include Game System (GS) header files.
// -----------------------------------------------------------------------------------------------
#include "gs_list.h"
#include "gs_ogl_texture.h"
#include "gs_ogl_sprite.h"
#include "gs_ogl_collide.h"
#include "gs_math.h"
//================================================================================================


//================================================================================================
// Include standard C library header files.
// -----------------------------------------------------------------------------------------------
#include <time.h>
//================================================================================================


//==============================================================================================
// Vine defines.
// ---------------------------------------------------------------------------------------------
#define MAX_SWING_SPEED      100.0f  // The higher this value, the smaller the swing arc
#define VINE_ELEMENT_PROX    4       // How close vine elements are together (the greater the number
                                     // the closer the elements are together i.e, 1/n)
#define VINE_SPRING_INTERVAL 10      // The interval of springs in the vine (e.g., every n-th element)
                                     // Smaller intervals equals more elastic vines (min: 1)
#define SPRING_TIGHTNESS     0.5f    // Tightness of the spring binding vine elements together
#define SPRING_DAMPING       0.0f    // Amount of damping for the spring binding elements together
//==============================================================================================



////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definition. ///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


class Vine
{

private:

    GS_OGLTexture m_vineSpriteTexture;
    GS_List<GS_OGLSprite*> m_vineSprites;

    GS_OGLCollide m_gsCollide;

    bool m_isCreated; // Whether the vine has been created or not

    int m_startX; // The X coordinate in pixels of the first element of the vine
    int m_startY; // The Y coordinate in pixels of the first element of the vine

    int m_swingMaxAmplitude; // The maximum amlitude of the vine swing in pixels
    int m_swingMinAmplitude; // The minimum amlitude of the vine swing in pixels
    int m_swingWidth;        // The total width of the swing in pixels
    int m_swingSpeed;        // How quickly the vine swings (1 - 100)

    int m_swingControlX1; // The X coordinate of the first control point for the swing
    int m_swingControlY1; // The Y coordinate of the first control point for the swing
    int m_swingControlX2; // The X coordinate of the second control point for the swing
    int m_swingControlY2; // The Y coordinate of the second control point for the swing

    int m_swingStartX; // The X coordinate where the swing arc will start
    int m_swingStartY; // The Y coordinate where the swing arc will start
    int m_swingEndX;   // The X coordinate where the swing arc will end
    int m_swingEndY;   // The Y coordinate where the swing arc will ned

    float m_swingMoveFrac = -0.1f; // The fraction with which the movement of the last
                                  // element of the vine will be adjusted

    float m_swingMoveAmt; // Combines the movement fraction and movement speed to determine
                          // the amount the last element of the vine will move

    // The elements of the vine are connected with spring physics. The following variables
    // determine the details of the aforementioned connections
    float m_tightness;
    float m_velocity;
    float m_damping;

    bool m_isSwingingLeft;

    double m_angle;

    void UpdateBezierMovement();
    void UpdateCircleMovement();

protected:

    // No protected members.

public:

    Vine();
    ~Vine();

    // Create the vine
    bool Create
    (
        int numSprites,
        int startX,
        int startY,
        float swingSpeed,
        int swingMaxAmplitude = -1,
        int swingMinAmplitude = -1,
        int swingWidth = -1
    );

    // Destroy the vine
    void Destroy();

    // Move the vine horizontally
    void AddDestX(int x);
    inline int GetDestX()
    {
        return m_startX;
    }

    // Get the element of the vine that collides with the source rectangle
    int GetCollideElement(RECT rcSource);

    void UpdateSwing( bool useBezier = true ); // Update the vine's swing

    bool Render(float fAlpha = 1.0f); // Render the vine

    // Get the number of elements in the vine
    inline int GetNumElements()
    {
        return m_vineSprites.GetNumItems();
    }

    // Get the x-coordinate of the specified element of the vine
    inline int GetElementDestX(int element)
    {
        // Is the element out of bounds?
        if( element >= m_vineSprites.GetNumItems() )
        {
            return false;
        }

        // Get the sprite in the list
        GS_OGLSprite* vineSprite = m_vineSprites.GetItem(element);

        // Set the source rectangle
        return vineSprite->GetDestX();
    }

    // Get the y-coordinate of the specified element of the vine
    inline int GetElementDestY(int element)
    {
        // Is the element out of bounds?
        if( element >= m_vineSprites.GetNumItems() )
        {
            return false;
        }

        // Get the sprite in the list
        GS_OGLSprite* vineSprite = m_vineSprites.GetItem(element);

        // Set the source rectangle
        return vineSprite->GetDestY();
    }

    // Get a rectangle containing the coordinates of the speciifed vine element
    inline bool GetElementRect(int element, RECT* pSourceRect)
    {
        // Is the element out of bounds?
        if( element >= m_vineSprites.GetNumItems() )
        {
            return false;
        }

        // Get the sprite in the list
        GS_OGLSprite* vineSprite = m_vineSprites.GetItem(element);

        // Set the destination rectangle
        vineSprite->GetDestRect(pSourceRect);

        return true;
    }

    // Get the angle of the swing
    inline double GetAngle()
    {
        return m_angle;
    }

    // Get the swing speed of the vine
    inline int GetSwingSpeed()
    {
        return m_swingSpeed;
    }

    // Set the swing speed of the vine
    inline void SetSwingSpeed( int speed )
    {
        m_swingSpeed = speed;
    }

    // Get whether the vine is swinging left or not
    inline bool IsSwingingLeft()
    {
        return m_isSwingingLeft;
    }

};

////////////////////////////////////////////////////////////////////////////////////////////////

#endif
