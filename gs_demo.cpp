/*============================================================================================+
 | Game System (GS) Library                                                                   |
 |--------------------------------------------------------------------------------------------|
 | FILES: gs_demo.cpp, gs_demo.h                                                              |
 |--------------------------------------------------------------------------------------------|
 | CLASS: GS_Demo                                                                             |
 |--------------------------------------------------------------------------------------------|
 | ABOUT: ...                                                                                 |
 |--------------------------------------------------------------------------------------------|
 |                                                                                    08/2003 |
 +============================================================================================*/


//==============================================================================================
// Include header files.
// ---------------------------------------------------------------------------------------------
#include "gs_demo.h"
//==============================================================================================


//==============================================================================================
// Global variables.
// ---------------------------------------------------------------------------------------------
GS_Demo g_gsDemo;
//==============================================================================================


////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTOR & DESTRUCTOR ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


//==============================================================================================
// GS_Demo::GS_Demo():
// ---------------------------------------------------------------------------------------------
// Purpose: The constructor, initializes class data to defaults when class object is created.
// ---------------------------------------------------------------------------------------------
// Returns: Nothing.
//==============================================================================================

GS_Demo::GS_Demo() : GS_Application()
{
    m_gsSettings.nDisplayWidth   = -1;
    m_gsSettings.nDisplayHeight  = -1;
    m_gsSettings.nColorDepth     = -1;
    m_gsSettings.bWindowedMode   = -1;
    m_gsSettings.bEnableVSync    = -1;
    m_gsSettings.bEnableAliasing = -1;
    m_gsSettings.nMusicVolume    = -1;
    m_gsSettings.nEffectsVolume  = -1;

    for (int nLoop = 0; nLoop < MAX_SCORES; nLoop++)
    {
        lstrcpy(m_gsHiscores[nLoop].szName, "..........");
        m_gsHiscores[nLoop].lScore  = 0l;
    }

    m_nGameProgress = 0;
    m_nPrevProgress = 0;
    m_nNextProgress = 0;

    m_bIsInitialized  = FALSE;
    m_bWasKeyReleased = FALSE;
    m_nOptionSelected = -1;
    m_nCounter = 0;

    m_fInterval = 0.0f;
    m_fRotate   = 0.0f;
    m_fScale    = 0.0f;
    m_fAlpha    = 0.0f;

    m_gsMenuTextColor.fRed   = 1.0f;
    m_gsMenuTextColor.fGreen = 1.0f;
    m_gsMenuTextColor.fBlue  = 1.0f;
    m_gsMenuTextColor.fAlpha = 0.3f;

    m_gsMenuHighColor.fRed   = 1.0f;
    m_gsMenuHighColor.fGreen = 1.0f;
    m_gsMenuHighColor.fBlue  = 1.0f;
    m_gsMenuHighColor.fAlpha = 1.0f;

    m_nScoreIndex = 0;
    m_lScore      = 0;

    m_nGameMode = GAME_MODE_NORMAL;

    m_nMaxVineElements = MAX_VINE_ELEMENTS;
    m_nMinVineElements = MIN_VINE_ELEMENTS;
    m_nMaxVineSpeed = MAX_VINE_SPEED;
    m_nMinVineSpeed = MIN_VINE_SPEED;
    m_nMaxHeroSwings = MAX_HERO_SWINGS;

    m_heroSampleIndex = 6;

    this->GameSetup();
}

//==============================================================================================
// GS_Demo::~GS_Demo():
// ---------------------------------------------------------------------------------------------
// Purpose: The de-structor, de-initializes class data when class object is destroyed.
// ---------------------------------------------------------------------------------------------
// Returns: Nothing.
//==============================================================================================

GS_Demo::~GS_Demo()
{

    // Does nothing at the moment.
}


////////////////////////////////////////////////////////////////////////////////////////////////
// GAME SETUP //////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


//==============================================================================================
// GS_Demo::GameSetup():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successful, FALSE if not.
//==============================================================================================

BOOL GS_Demo::GameSetup()
{
    // Were we not able to load game settings?
    if (FALSE == this->LoadSettings())
    {
        // Display a message allowing the user to choose full screen or windowed mode.
        if (MessageBox(NULL, "Would you like to run in fullscreen mode?", "PLEASE SELECT",
                       MB_YESNO | MB_ICONQUESTION) == IDYES)
        {
            // Run game in fullscreen mode.
            m_gsSettings.bWindowedMode = FALSE;
        }
        else
        {
            // Run game in windowed mode.
            m_gsSettings.bWindowedMode = TRUE;
        }

        // Set all other game settings to their defaults.
        m_gsSettings.nDisplayWidth   = DEFAULT_WIDTH;
        m_gsSettings.nDisplayHeight  = DEFAULT_HEIGHT;
        m_gsSettings.nColorDepth     = DEFAULT_DEPTH;
        m_gsSettings.bEnableVSync    = DEFAULT_VSYNC;
        m_gsSettings.bEnableAliasing = DEFAULT_ALIAS;
        m_gsSettings.nMusicVolume    = DEFAULT_MUSIC;
        m_gsSettings.nEffectsVolume  = DEFAULT_SOUND;
    }

    // Set the display mode
    this->SetMode
    (
        m_gsSettings.nDisplayWidth,
        m_gsSettings.nDisplayHeight,
        m_gsSettings.nColorDepth,
        m_gsSettings.bWindowedMode
    );

    // Set render scaling values
    this->SetRenderScaling(this->GetWidth(), this->GetHeight(), true);

    // Load the game hiscores.
    this->LoadHiscores();

    // Let the game run as fast as it can
    this->SetFrameRate(0.0f);

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////
// INITIALIZE & SHUTDOWN ///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


//==============================================================================================
// GS_Demo::GameInit():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successful, FALSE if not.
//==============================================================================================

BOOL GS_Demo::GameInit()
{
    // Seed the timer for random numbers.
    time_t t;
    srand((unsigned) time(&t));

    // Setup the text for the window title bar.
    this->SetTitle("VineHero Concept Demo");

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Create And Initialize The OpenGL Display /////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Did we fail creating the OpenGL display?
    if ( !m_gsDisplay.Create(this->GetDevice(), this->GetWidth(), this->GetHeight(), this->GetColorDepth()) )
    {
        GS_Error::Report("GS_DEMO.CPP", 186, "Failed to create OpenGL display!");
        return FALSE;
    }

    // Enable vertical sync if specified in the game settings.
    m_gsDisplay.EnableVSync(m_gsSettings.bEnableVSync);

    // Enable anti-aliasing if specified in the game settings.
    m_gsDisplay.EnableAliasing(m_gsSettings.bEnableAliasing);

    // Enable blending for transparency in textures.
    m_gsDisplay.EnableBlending(TRUE);

    // Are we in fullscreen mode?
    // if (!this->IsWindowed())
    // {
        // Hide the mouse cursor.
        m_gsMouse.HideCursor(TRUE);
    // }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Initialize The FMOD Sound System /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Did we fail to initialize the FMOD sound system?
    if (!m_gsSound.Initialize(GetWindow()))
    {
        GS_Error::Report("GS_DEMO.CPP", 211, "Failed to initialize FMOD sound!");
        return FALSE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Custom Graphics Initialization ////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Create an unfiltered sprite that will be used for the background.
    BOOL bResult = m_gsBackSprite.Create("data/background.tga", FALSE);

    // Were we not successful?
    if (FALSE == bResult)
    {
        GS_Error::Report("GS_DEMO.CPP", 224, "Failed to create background sprite!");
    }

    // Set the source rectangle of the background sprite to 856x480.
    m_gsBackSprite.SetSourceRect(0, INTERNAL_RES_Y, INTERNAL_RES_X, 0);
    // Postion the background at the bottom left of the screen.
    m_gsBackSprite.SetDestXY(0, 0);

    // Create an unfiltered sprite that will be used for the cursor.
    bResult = m_gsCursorSprite.Create("data/cursor.tga", FALSE);

    // Were we not successful?
    if (FALSE == bResult)
    {
        GS_Error::Report("GS_DEMO.CPP", 237, "Failed to create cursor sprite!");
    }

    // Set up a rectangle the size of the screen for rendering tiles.
    SetRect(&m_rcScreen, 0, INTERNAL_RES_Y, INTERNAL_RES_X, 0);

    // Create an unfiltered sprite that will be used for the title.
    bResult = m_gsTitleSprite.Create("data/title.tga", FALSE);

    // Were we not successful?
    if (FALSE == bResult)
    {
        GS_Error::Report("GS_DEMO.CPP", 256, "Failed to create title sprite!");
    }

    // Create an unfiltered mipmap texture from the "font_01.tga" image, for the game font.
    bResult = m_gsFontTexture.Create("data/font.tga", GS_MIPMAP, GL_NEAREST, GL_NEAREST);

    // Were we not successful?
    if (FALSE == bResult)
    {
        GS_Error::Report("GS_DEMO.CPP", 264, "Failed to create font texture!");
    }

    // Use the font texture to create a font with 16 characters per line and with a width
    // and height of 16x16 pixels per character.
    bResult = m_gsFont.Create(m_gsFontTexture.GetID(), m_gsFontTexture.GetWidth(),
                              m_gsFontTexture.GetHeight(), 20, 16, 16);

    // Were we not successful?
    if (FALSE == bResult)
    {
        GS_Error::Report("GS_DEMO.CPP", 274, "Failed to create font!");
    }

    // Create a menu using the "menu_01.tga" image and the font texture created earlier.
    bResult = m_gsMenu.Create("data/menu.tga", m_gsFontTexture.GetID(),
                              m_gsFontTexture.GetWidth(), m_gsFontTexture.GetHeight(),
                              20, 16, 16);

    // Were we not successful?
    if (FALSE == bResult)
    {
        GS_Error::Report("GS_DEMO.CPP", 284, "Failed to create game menu!");
    }

    // Create a hero select menu using the "menu_01.tga" image and the font texture created earlier.
    bResult = m_gsHeroSelectMenu.Create("data/menu.tga", m_gsFontTexture.GetID(),
                              m_gsFontTexture.GetWidth(), m_gsFontTexture.GetHeight(),
                              20, 16, 16);

    // Were we not successful?
    if (FALSE == bResult)
    {
        GS_Error::Report("GS_DEMO.CPP", 284, "Failed to create select hero menu!");
    }

    // Get randomised swing speed
    m_vineLastSwingSpeed = rand()%(MAX_VINE_SPEED-MIN_VINE_SPEED) + MIN_VINE_SPEED;

    // Create the first vine
    m_vines[0].Create
    (
        rand()%(MAX_VINE_ELEMENTS-MIN_VINE_ELEMENTS) + MIN_VINE_ELEMENTS, // number of elements
        0,                      // start x
        INTERNAL_RES_Y - 10,    // start y
        m_vineLastSwingSpeed,   // swing speed
        -1,                     // max amplitude of swing
        -1,                     // min amplitude of swing
        -1                      // swing width
    );

    // Create all additional vines
    for( int nLoop = 1; nLoop < MAX_VINES; nLoop++)
    {
        // Make certain the swing speed for the next vine is different
        int newSwingSpeed = rand()%(MAX_VINE_SPEED-MIN_VINE_SPEED) + MIN_VINE_SPEED;
        while( (newSwingSpeed > m_vineLastSwingSpeed - 5) && (newSwingSpeed < m_vineLastSwingSpeed + 5) )
        {
            newSwingSpeed = rand()%(MAX_VINE_SPEED-MIN_VINE_SPEED) + MIN_VINE_SPEED;
        }
        m_vineLastSwingSpeed = newSwingSpeed;

        // Create the vine
        m_vines[nLoop].Create
        (
            rand()%(MAX_VINE_ELEMENTS-MIN_VINE_ELEMENTS) + MIN_VINE_ELEMENTS, // number of elements
            INTERNAL_RES_X - (INTERNAL_RES_X / 2),  // start x
            INTERNAL_RES_Y - 10,                    // start y
            m_vineLastSwingSpeed,                   // swing speed
            -1,                                     // max amplitude of swing
            -1,                                     // min amplitude of swing
            -1                                      // swing width
        );
    }

    // Set vine default values
    m_vineIndex = MAX_VINES - 1;
    m_vineElementIndex = m_vines[m_vineIndex].GetNumElements() - 4;
    m_vineSwingSamplePlayed = false;

    // Create the hero
    m_hero.Create(m_hero.GetSelectedHero(), -1, -1, true);
    m_heroSampleIndex = m_hero.GetSelectedHero() * NUM_HERO_SAMPLES;

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Custom Sound Initialization ///////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Add the game music to a list.
    m_gsSound.AddStream("data/music_title.mp3");
    m_gsSound.AddStream("data/music_game.mp3");

    // Add the game samples to list.
    m_gsSound.AddSample("data/menu_navigate.wav");
    m_gsSound.AddSample("data/menu_select.wav");
    m_gsSound.AddSample("data/vine_swoosh.mp3");
    m_gsSound.AddSample("data/hero_01_jump.mp3");
    m_gsSound.AddSample("data/hero_01_death.mp3");
    m_gsSound.AddSample("data/hero_01_death_quote_01.mp3");
    m_gsSound.AddSample("data/hero_01_death_quote_02.mp3");
    m_gsSound.AddSample("data/hero_01_death_quote_03.mp3");
    m_gsSound.AddSample("data/hero_01_hiscore_quote.mp3");
    m_gsSound.AddSample("data/hero_02_jump.mp3");
    m_gsSound.AddSample("data/hero_02_death.mp3");
    m_gsSound.AddSample("data/hero_02_death_quote_01.mp3");
    m_gsSound.AddSample("data/hero_02_death_quote_02.mp3");
    m_gsSound.AddSample("data/hero_02_death_quote_03.mp3");
    m_gsSound.AddSample("data/hero_02_hiscore_quote.mp3");

    // Set the master volume for all the game sound effects.
    m_gsSound.SetSampleMaster(m_gsSettings.nEffectsVolume);

    // Are we already playing the game?
    if ((m_nGameProgress >= PLAY_GAME) && (m_nGameProgress < PLAY_OUTRO))
    {
        // Start playing game music at the specified volume, 1x speed and looping infinitely.
        m_gsSound.PlayStream(MUSIC_GAME, m_gsSettings.nMusicVolume);
    }
    else if (m_nGameProgress > GAME_INTRO)
    {
        // Start playing title music at the specified volume, 1x speed and looping infinitely.
        m_gsSound.PlayStream(MUSIC_TITLE, m_gsSettings.nMusicVolume); //(m_gsSettings.nMusicVolume + 100 <= 255) ? m_gsSettings.nMusicVolume + 100 : 255);
    }

    return TRUE;
}

//==============================================================================================
// GS_Demo::GameShutdown():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successful, FALSE if not.
//==============================================================================================

BOOL GS_Demo::GameShutdown()
{

    // Reset keyboard data.
    m_gsKeyboard.Reset();

    // Reset mouse data.
    m_gsMouse.Reset();

    // Destroy the OpenGL display.
    m_gsDisplay.Destroy();

    // Shutdown the sound system.
    m_gsSound.Shutdown();

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////
// FOCUS METHODS ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


//==============================================================================================
// GS_Demo::GameRelease():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successful, FALSE if not.
//==============================================================================================

BOOL GS_Demo::GameRelease()
{

    // Is the application not active?
    if (!this->IsActive())
    {
        return TRUE;
    }

    // Set OpenGL clear color to black.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Clear screen and depth buffer.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset the current modelview matrix.
    glLoadIdentity();

    // Swap buffers (double buffering) to display results.
    SwapBuffers(this->GetDevice());

    // Pause all game sound.
    m_gsSound.Pause(TRUE);

    // Pause the game.
    this->Pause(TRUE);

    return TRUE;
}

//==============================================================================================
// GS_Demo::GameRestore():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successful, FALSE if not.
//==============================================================================================

BOOL GS_Demo::GameRestore()
{

    // Is the application active?
    if (this->IsActive())
    {
        return TRUE;
    }

    // Unpause all sound.
    m_gsSound.Pause(FALSE);

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////
// THE GAME LOOP ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


//==============================================================================================
// GS_Demo::GameLoop():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successful, FALSE if not.
//==============================================================================================

BOOL GS_Demo::GameLoop()
{
    // Run method depending on progress.
    switch (m_nGameProgress)
    {
        // Game intro & outro methods.
        case GAME_INTRO:
            return GameIntro();
        case GAME_OUTRO:
            return GameOutro();

        // Title screen methods.
        case TITLE_INTRO:
            return TitleIntro();
        case TITLE_SCREEN:
            return TitleScreen();
        case TITLE_OUTRO:
            return TitleOutro();

        // Option screen methods.
        case OPTION_INTRO:
            return OptionIntro();
        case OPTION_SCREEN:
            return OptionScreen();
        case OPTION_OUTRO:
            return OptionOutro();

        // Game play methods.
        case PLAY_INTRO:
            return PlayIntro();
        case PLAY_GAME:
            return PlayGame();
        case PLAY_UPDATE:
            return PlayUpdate();
        case PLAY_PAUSE:
            return PlayPause();
        case PLAY_EXIT:
            return PlayExit();
        case PLAY_OUTRO:
            return PlayOutro();

        // Hiscore methods.
        case SCORES_INTRO:
            return ScoresIntro();
        case SCORES_VIEW:
            return ScoresView();
        case SCORES_ADD:
            return ScoresAdd();
        case SCORES_OUTRO:
            return ScoresOutro();

        // No valid progress?
        default:
            m_nGameProgress = GAME_OUTRO;
            break;
    } // end switch

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////
// MESSAGE HANDLING ////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


//==============================================================================================
// GS_Demo::OnChangeMode():
// ---------------------------------------------------------------------------------------------
// Purpose: Overrides the OnChangeMode() function of the base class
// ---------------------------------------------------------------------------------------------
// Returns: Nothing
//==============================================================================================

void GS_Demo::OnChangeMode()
{
    // Change to windowed or fullscreen mode as selected by the user
    this->SetMode
    (
        m_gsSettings.nDisplayWidth,
        m_gsSettings.nDisplayHeight,
        this->GetColorDepth(),
        !this->IsWindowed()
    );

    // Set values for scaling
    this->SetRenderScaling(this->GetWidth(), this->GetHeight(), true);

    // Has the mode change been successful?
    m_gsSettings.bWindowedMode = this->IsWindowed();

    // Save game settings.
    this->SaveSettings();
}

//==============================================================================================
// GS_Demo::MsgProc():
// ---------------------------------------------------------------------------------------------
// Purpose: Overrides the main WndProc() and MsgProc() of the base class, so the application
//          can do custom message handling (e.g. processing mouse, keyboard, or menu commands).
// ---------------------------------------------------------------------------------------------
// Returns: LRESULT depending on the message processed.
//==============================================================================================

LRESULT GS_Demo::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    switch (uMsg)
    {

    // Received when a WM_KEYDOWN message is translated by the TranslateMessage()
    // function. WM_CHAR contains the character code of the key that was pressed.
    case WM_CHAR:
        // wParam contains information on which key was pressed.
        switch (wParam)
        {
        // User pressed 'b' or 'B'.
        case 'B':
        case 'b':
            // Are at the not at the ScoresAdd() method?
            if (m_nGameProgress != SCORES_ADD)
            {
                // Is alpha blending enabled?
                if (m_gsDisplay.IsBlendingEnabled())
                {
                    // Disable alpha blending.
                    m_gsDisplay.EnableBlending(FALSE);
                }
                else
                {
                    // Enable alpha blending.
                    m_gsDisplay.EnableBlending(TRUE);
                }
            }
            break;
        // User pressed 'p' or 'P'.
        case 'P':
        case 'p':
            // Is the application pause?
            if (this->IsPaused())
            {
                // Unpause the application.
                this->Pause(FALSE);
            }
            else
            {
                // Pause the application.
                this->Pause(TRUE);
            }
            break;
        // User pressed 't' or 'T'.
        case 'T':
        case 't':
            // Are at the not at the ScoresAdd() method?
            if (m_nGameProgress != SCORES_ADD)
            {
                // Is the frame rate greater than 0?
                if (this->GetFrameRate() > 0.0f)
                {
                    this->SetFrameRate(0.0f);
                }
                else
                {
                    this->SetFrameRate(60.0f);
                }
            }
            break;
        // User pressed 'v' or 'V'.
        case 'V':
        case 'v':
            // Are at the not at the ScoresAdd() method?
            if (m_nGameProgress != SCORES_ADD)
            {
                // Is vertical sync enabled?
                if (m_gsDisplay.IsVSyncEnabled())
                {
                    // Disable vertical sync.
                    m_gsDisplay.EnableVSync(FALSE);
                }
                else
                {
                    // Enable vertical sync.
                    m_gsDisplay.EnableVSync(TRUE);
                }
            }
            break;
        // Default processing.
        default:
            break;
        } // end switch (wParam...
        break;

    // Received when a nonsystem key is pressed. A nonsystem key is a key
    // that is pressed when the ALT key is not pressed.
    case WM_KEYDOWN:
        // Mark the key that was pressed.
        m_gsKeyboard.KeyDown(wParam);
        // Add the key to the keyboard buffer.
        m_gsKeyboard.AddKeyToBuffer(wParam);
        break;

    // Received when a nonsystem key is released. A nonsystem key is a key
    // that is pressed  when the ALT key is not pressed.
    case WM_KEYUP:
        // Mark the key that was released.
        m_gsKeyboard.KeyUp(wParam);
        break;

    // Received when the user holds down the ALT key and then presses another key.
    case WM_SYSKEYDOWN:
        // Mark the key that was pressed.
        m_gsKeyboard.KeyDown(wParam);
        // Add the key to the keyboard buffer.
        m_gsKeyboard.AddKeyToBuffer(wParam);
        return 0L;

    // Received when the user releases a key that was pressed while the ALT key held down.
    case WM_SYSKEYUP:
        // Mark the key that was released.
        m_gsKeyboard.KeyUp(wParam);
        return 0L;

    // Received when the cursor moves within a window and mouse input is not captured.
    case WM_SETCURSOR:
        // Are we in windowed mode?
        if (this->IsWindowed())
        {
            // Is the mouse cursor in the window client?
            if (m_gsMouse.IsInClient(hWnd))
            {
                // Hide the cursor.
                SetCursor(NULL);
                return TRUE;
            }
        }
        break;

    // The WM_MOUSEMOVE message is posted to a window when the cursor moves.
    case WM_MOUSEMOVE:
        // Save mouse coordinates (coordinates are relative to the top left of the client
        // area of the window). Note that the Y coordinates are inverted for OpenGL.
        m_gsMouse.SetX(LOWORD(lParam));
        m_gsMouse.SetY(INTERNAL_RES_Y - HIWORD(lParam) - 1);
        return 0L;

    // The WM_LBUTTONDOWN message is posted when the user presses the left mouse
    // button while/ the cursor is in the client area of a window.
    case WM_LBUTTONDOWN:
        // The left mouse button is down.
        m_gsMouse.LeftPressed(TRUE);
        return 0L;

    // The WM_MBUTTONDOWN message is posted when the user presses the middle
    // mouse button while the cursor is in the client area of a window.
    case WM_MBUTTONDOWN:
        // The middle mouse button is down.
        m_gsMouse.MiddlePressed(TRUE);
        return 0L;

    // The WM_RBUTTONDOWN message is posted when the user presses the right
    // mouse button while the cursor is in the client area of a window.
    case WM_RBUTTONDOWN:
        // The right mouse button is down.
        m_gsMouse.RightPressed(TRUE);
        return 0L;

    // The WM_LBUTTONUP message is posted when the user releases the left mouse
    // button while the cursor is in the client area of a window.
    case WM_LBUTTONUP:
        // The left mouse button is up.
        m_gsMouse.LeftPressed(FALSE);
        return 0L;

    // The WM_MBUTTONUP message is posted when the user releases the middle
    // mouse button while the cursor is in the client area of a window.
    case WM_MBUTTONUP:
        // The middle mouse button is up.
        m_gsMouse.MiddlePressed(FALSE);
        return 0L;

    // The WM_RBUTTONUP message is posted when the user releases the right
    // mouse button while the cursor is in the client area of a window.
    case WM_RBUTTONUP:
        // The right mouse button is up.
        m_gsMouse.RightPressed(FALSE);
        return 0L;
    } // end switch (uMsg...

    // Provide default processing by the base class.
    return GS_Application::MsgProc(hWnd, uMsg, wParam, lParam);
}


////////////////////////////////////////////////////////////////////////////////////////////////
// INTRO & OUTRO METHODS ///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


//==============================================================================================
// GS_Demo::GameIntro():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successfull and FALSE if not.
//==============================================================================================

BOOL GS_Demo::GameIntro()
{

    // Has the display not been initialized?
    if (!this->m_gsDisplay.IsReady())
    {
        return FALSE;
    }

    // Has the game been paused?
    if (this->IsPaused())
    {
        // Unpause the game.
        this->Pause(FALSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Initialization /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Has the method not been initialized?
    if (m_bIsInitialized == FALSE)
    {
        // Initialize variables.
        m_fInterval = 0.0f;
        m_nCounter  = 0;
        m_fAlpha    = 0.0f;
        // Clear the keyboard buffer.
        m_gsKeyboard.ClearBuffer();
        // Where we want to go to next.
        m_nNextProgress = TITLE_INTRO;
        // Initialization completed.
        m_bIsInitialized = TRUE;
        // Return to prevent the time it took to initialize the game, which constitutes
        // the first measured frame, from affecting the timing of the game.
        return TRUE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Logic //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Determine the interval required to perform an action 60 times every second at the
    // current frame rate, and add it to the previous intervals.
    m_fInterval += this->GetActionInterval(60);

    // Have all the intervals added up enough to perform an action?
    while (m_fInterval >= 1.0f)
    {
        // Increase counter.
        m_nCounter++;
        // Act depending on how many counts have elapsed.
        if ((m_nCounter >= 50) && (m_nCounter < 150))
        {
            // Decrease transparency.
            m_fAlpha += 0.025;
        }
        else if ((m_nCounter >= 150) && (m_nCounter < 250))
        {
            // Increase transparency.
            m_fAlpha -= 0.025;
        }
        else if (m_nCounter >= 250)
        {
            // Progress to the next section.
            m_nGameProgress = m_nNextProgress;
        }
        // One action less to be taken.
        m_fInterval -= 1.0f;
    }

    // Has the user pressed a key?
    if (m_gsKeyboard.GetBufferedKey() > 0)
    {
        // Progress to the next section.
        m_nGameProgress = m_nNextProgress;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Rendering //////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Clear the screen to the specified color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset display.
    glLoadIdentity();

    // Setup the display for rendering in 2D.
    m_gsDisplay.BeginRender2D(this->GetWindow());

    // Set the alpha value (transparency) for the font.
    m_gsFont.SetModulateColor(-1.0, -1.0, -1.0, m_fAlpha);

    // Display a message centered in the screen.
    m_gsFont.SetText("LEXPRO UNDERGROUND");
    m_gsFont.SetDestX((INTERNAL_RES_X -  m_gsFont.GetTextWidth()) / 2);
    m_gsFont.SetDestY(((INTERNAL_RES_Y - m_gsFont.GetTextHeight()) / 2) +
                      m_gsFont.GetTextHeight());
    m_gsFont.Render();

    m_gsFont.SetText("PRESENTS");
    m_gsFont.SetDestX((INTERNAL_RES_X -  m_gsFont.GetTextWidth()) / 2);
    m_gsFont.SetDestY(((INTERNAL_RES_Y - m_gsFont.GetTextHeight()) / 2) -
                      m_gsFont.GetTextHeight());
    m_gsFont.Render();

    // Reset display after rendering in 2D.
    m_gsDisplay.EndRender2D();

    // Swap buffers to display results.
    SwapBuffers(this->GetDevice());

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Cleanup ////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Are we leaving this section?
    if (m_nGameProgress != GAME_INTRO)
    {
        // Remember where we've come from.
        m_nPrevProgress = GAME_INTRO;

        // Is title music not already playing?
        if (!m_gsSound.IsStreamPlaying())
        {
            // Start playing title music at the specified volume, 1x speed and looping infinitely.
            m_gsSound.PlayStream(MUSIC_TITLE, m_gsSettings.nMusicVolume); //(m_gsSettings.nMusicVolume + 100 <= 255) ? m_gsSettings.nMusicVolume + 100 : 255);
        }

        // Reset method variables.
        m_bIsInitialized = FALSE;
    }

    return TRUE;
}

//==============================================================================================
// GS_Demo::GameOutro():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successfull and FALSE if not.
//==============================================================================================

BOOL GS_Demo::GameOutro()
{

    // Has the display not been initialized?
    if (!this->m_gsDisplay.IsReady())
    {
        return FALSE;
    }

    // Has the game been paused?
    if (this->IsPaused())
    {
        // Unpause the game.
        this->Pause(FALSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Initialization /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Has the method not been initialized?
    if (m_bIsInitialized == FALSE)
    {
        // Clear the keyboard buffer.
        m_gsKeyboard.ClearBuffer();
        // Initialize method variables.
        m_fInterval = 0.0f;
        m_nCounter  = 0;
        m_fAlpha    = 1.0f;
        // Is the title music playing?
        if (m_gsSound.IsStreamPlaying())
        {
            // Stop title music from playing.
            m_gsSound.StopStream();
        }
        // Initialization completed.
        m_bIsInitialized = TRUE;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////
    // Get Keyboard Input ///////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Has the user pressed a key?
    if (m_gsKeyboard.GetBufferedKey() > 0)
    {
        // Quit the application.
        this->Quit();
        return TRUE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Logic //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Determine the interval required to perform an action 60 times every second at the
    // current frame rate, and add it to the previous intervals.
    m_fInterval += this->GetActionInterval(60);

    // Have all the intervals added up enough to perform an action?
    while (m_fInterval >= 1.0f)
    {
        // Increase counter.
        m_nCounter++;
        // Act depending on how many counts have elapsed.
        if ((m_nCounter >= 350) && (m_nCounter < 400))
        {
            // Increase transparency.
            m_fAlpha -= 0.025;
        }
        else if (m_nCounter >= 400)
        {
            // Quit the application.
            this->Quit();
            return TRUE;
        }
        // One action less to be taken.
        m_fInterval -= 1.0f;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Rendering //////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Clear the screen to the specified color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset display.
    glLoadIdentity();

    // Setup the display for rendering in 2D.
    m_gsDisplay.BeginRender2D(this->GetWindow());

    // Display the game credits centered in the screen.
    m_gsFont.SetText("CREDITS");
    m_gsFont.SetScaleXY(2.0f, 2.0f);
    m_gsFont.SetDestX((INTERNAL_RES_X - m_gsFont.GetTextWidth())  / 2);
    m_gsFont.SetDestY(432);
    m_gsFont.SetModulateColor(1.0f, 1.0f, 1.0f, m_fAlpha);
    m_gsFont.Render();

    m_gsFont.SetText("DEVELOPED BY");
    m_gsFont.SetScaleXY(1.0f, 1.0f);
    m_gsFont.SetDestX((INTERNAL_RES_X - m_gsFont.GetTextWidth())  / 2);
    m_gsFont.SetDestY(384);
    m_gsFont.SetModulateColor(1.0f, 1.0f, 1.0f, m_fAlpha);
    m_gsFont.Render();

    m_gsFont.SetText("LEXPRO SYSTEMS");
    m_gsFont.SetDestX((INTERNAL_RES_X - m_gsFont.GetTextWidth())  / 2);
    m_gsFont.SetDestY(352);
    m_gsFont.SetModulateColor(1.0f, 0.5f, 0.5f, m_fAlpha);
    m_gsFont.Render();

    m_gsFont.SetText("DEVEOLOPERS:");
    m_gsFont.SetDestX((INTERNAL_RES_X - m_gsFont.GetTextWidth())  / 2);
    m_gsFont.SetDestY(304);
    m_gsFont.SetModulateColor(1.0f, 1.0f, 1.0f, m_fAlpha);
    m_gsFont.Render();

    m_gsFont.SetText("ANDRIES.....    BEN.........");
    m_gsFont.SetDestX((INTERNAL_RES_X - m_gsFont.GetTextWidth())  / 2);
    m_gsFont.SetDestY(272);
    m_gsFont.SetModulateColor(1.0f, 0.5f, 0.5f, m_fAlpha);
    m_gsFont.Render();

    m_gsFont.SetText("ETIENNE.....    RUAN........");
    m_gsFont.SetDestX((INTERNAL_RES_X - m_gsFont.GetTextWidth())  / 2);
    m_gsFont.SetDestY(240);
    m_gsFont.Render();

    m_gsFont.SetText("WILLIAM.....    ............");
    m_gsFont.SetDestX((INTERNAL_RES_X - m_gsFont.GetTextWidth())  / 2);
    m_gsFont.SetDestY(208);
    m_gsFont.Render();

    m_gsFont.SetText("............    ............");
    m_gsFont.SetDestX((INTERNAL_RES_X - m_gsFont.GetTextWidth())  / 2);
    m_gsFont.SetDestY(176);
    m_gsFont.Render();

    m_gsFont.SetText("............    ............");
    m_gsFont.SetDestX((INTERNAL_RES_X - m_gsFont.GetTextWidth())  / 2);
    m_gsFont.SetDestY(144);
    m_gsFont.Render();

    m_gsFont.SetText("THANKS FOR PLAYING!");
    m_gsFont.SetDestX((INTERNAL_RES_X - m_gsFont.GetTextWidth())  / 2);
    m_gsFont.SetDestY(80);
    m_gsFont.SetModulateColor(1.0f, 1.0f, 1.0f, m_fAlpha);
    m_gsFont.Render();

    m_gsFont.SetText("PRESS ANY KEY TO EXIT");
    m_gsFont.SetDestX((INTERNAL_RES_X - m_gsFont.GetTextWidth())  / 2);
    m_gsFont.SetDestY(32);
    m_gsFont.SetModulateColor(1.0f, 1.0f, 1.0f, m_fAlpha);
    m_gsFont.Render();

    // Reset display after rendering in 2D.
    m_gsDisplay.EndRender2D();

    // Swap buffers to display results.
    SwapBuffers(this->GetDevice());

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Cleanup ////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Are we leaving this section?
    if (m_nGameProgress != GAME_OUTRO)
    {
        // Remember where we've come from.
        m_nPrevProgress = GAME_OUTRO;
        // Reset method variables.
        m_bIsInitialized = FALSE;
    }

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////
// TITLE SCREEN METHODS ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


//==============================================================================================
// GS_Demo::TitleIntro():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successfull and FALSE if not.
//==============================================================================================

BOOL GS_Demo::TitleIntro()
{

    // Has the display not been initialized?
    if (!this->m_gsDisplay.IsReady())
    {
        return FALSE;
    }

    // Has the game been paused?
    if (this->IsPaused())
    {
        // Unpause the game.
        this->Pause(FALSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Initialization /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Has the method not been initialized?
    if (m_bIsInitialized == FALSE)
    {
        // Initialize method variables.
        m_fAlpha    = 0.0f;
        // Where we want to go to next.
        m_nNextProgress = TITLE_SCREEN;
        // Initialization completed.
        m_bIsInitialized = TRUE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Logic //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Are images not entirely solid?
    if (m_fAlpha < 1.0f)
    {
        // Decrease transparency.
        m_fAlpha += this->GetActionInterval(60 * 0.025f);
    }
    else
    {
        // Progress to the next section.
        m_nGameProgress = m_nNextProgress;
        // No transparency.
        m_fAlpha = 1.0f;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Rendering //////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Clear the screen to the specified color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset display.
    glLoadIdentity();

    // Setup the display for rendering in 2D.
    m_gsDisplay.BeginRender2D(this->GetWindow());

    // Render the background image.
    this->RenderBackground(m_fAlpha);

    // Reset display after rendering in 2D.
    m_gsDisplay.EndRender2D();

    // Swap buffers to display results.
    SwapBuffers(this->GetDevice());

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Cleanup ////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Are we leaving this section?
    if (m_nGameProgress != TITLE_INTRO)
    {
        // Remember where we've come from.
        m_nPrevProgress = TITLE_INTRO;
        // Reset method variables.
        m_bIsInitialized = FALSE;
    }

    return TRUE;
}

//==============================================================================================
// GS_Demo::TitleScreen():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successfull and FALSE if not.
//==============================================================================================

BOOL GS_Demo::TitleScreen()
{

    // Has the display not been initialized?
    if (!this->m_gsDisplay.IsReady())
    {
        return FALSE;
    }

    // Has the game been paused?
    if (this->IsPaused())
    {
        // Unpause the game.
        this->Pause(FALSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Initialization /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Has the method not been initialized?
    if (m_bIsInitialized == FALSE)
    {
        // Initialize method variables.
        m_nOptionSelected = -1;
        m_fInterval = 0.0f;
        // Clear the keyboard buffer.
        m_gsKeyboard.ClearBuffer();
        // Setup variables for mouse input.
        m_nOldMouseX = m_gsMouse.GetX();
        m_nOldMouseY = m_gsMouse.GetY();
        m_bWasMouseReleased = FALSE;
        // Clear all current menu items.
        m_gsMenu.ClearOptions();
        // Menu don't have a title.
        m_gsMenu.SetTitle("");
        // Add the menu options.
        m_gsMenu.AddOption("PLAY GAME    ");
        m_gsMenu.AddOption("VIEW HISCORES");
        m_gsMenu.AddOption("SET OPTIONS  ");
        m_gsMenu.AddOption("QUIT         ");
        // Highlight the first option.
        m_gsMenu.SetHighlight(0);
        // Is the title music not already playing?
        if (!m_gsSound.IsStreamPlaying())
        {
            // Start playing title music at the specified volume, 1x speed and looping infinitely.
            m_gsSound.PlayStream(MUSIC_TITLE, m_gsSettings.nMusicVolume); //(m_gsSettings.nMusicVolume + 100 <= 255) ? m_gsSettings.nMusicVolume + 100 : 255);
        }
        // Initialization completed.
        m_bIsInitialized = TRUE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Get Keyboard Input ///////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Check to see wether a key was pressed.
    int nKey = m_gsKeyboard.GetBufferedKey();

    // Act depending on key pressed.
    switch (nKey)
    {
    // Was the up key pressed?
    case GSK_UP:
        // Highlight the previous option.
        m_gsMenu.HighlightPrev();
        // Play the appropriate sound effect.
        m_gsSound.PlaySample(SAMPLE_OPTION);
        // Reset time counter.
        m_fInterval = 0.0f;
        break;
    // Was the down key pressed?
    case GSK_DOWN:
        // Highlight the next option.
        m_gsMenu.HighlightNext();
        // Play the appropriate sound effect.
        m_gsSound.PlaySample(SAMPLE_OPTION);
        // Reset time counter.
        m_fInterval = 0.0f;
        break;
    // Was the enter key pressed?
    case GSK_ENTER:
        // Remeber which option is highlighted.
        m_nOptionSelected = m_gsMenu.GetHighlight();
        // Play the appropriate sound effect.
        m_gsSound.PlaySample(SAMPLE_SELECT);
        // Reset time counter.
        m_fInterval = 0.0f;
        break;
    // Was the escape key pressed?
    case GSK_ESCAPE:
        // Have the quit option been highlighted?
        if (m_gsMenu.GetHighlight() == 3)
        {
            // Set game progress to quit game.
            m_nNextProgress = GAME_OUTRO;
            m_nGameProgress = TITLE_OUTRO;
        }
        else
        {
            // Highlight the quit option.
            m_gsMenu.SetHighlight(3);
        }
        // Play the appropriate sound effect.
        m_gsSound.PlaySample(SAMPLE_OPTION);
        // Reset time counter.
        m_fInterval = 0.0f;
        break;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Get Mouse Input //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    RECT rcOption;

    // Has the mouse cursor moved from its previous position?
    if ((m_nOldMouseX != m_gsMouse.GetX()) || (m_nOldMouseY != m_gsMouse.GetY()))
    {
        // Check each option to see if the mouse coordinates are within the option area.
        for (int nLoop = 0; nLoop < m_gsMenu.GetNumOptions(); nLoop++)
        {
            // Determine the screen coordinates of the next option.
            m_gsMenu.GetOptionRect(nLoop, &rcOption);
            // Are the mouse coordinates within the option area?
            if (m_gsCollide.IsCoordInRect(m_gsMouse.GetX(), m_gsMouse.GetY(), rcOption))
            {
                // Is the option not highlighted already?
                if (nLoop != m_gsMenu.GetHighlight())
                {
                    // Highlight the selected option.
                    m_gsMenu.SetHighlight(nLoop);
                    // Play the appropriate sound effect.
                    m_gsSound.PlaySample(SAMPLE_OPTION);
                }
            }
        }
        // Assign new mouse coordinates.
        m_nOldMouseX = m_gsMouse.GetX();
        m_nOldMouseY = m_gsMouse.GetY();
        // Reset time counter.
        m_fInterval = 0.0f;
    }

    // Is the left mouse button not pressed?
    if (!m_gsMouse.IsLeftPressed())
    {
        m_bWasMouseReleased = TRUE;
    }

    // Was the left mouse button pressed?
    if ((m_gsMouse.IsLeftPressed()) && (m_bWasMouseReleased))
    {
        // Determine the screen coordinates of the highlighted option.
        m_gsMenu.GetHighlightRect(&rcOption);
        // Check if mouse coordinates are within the area of the highligted option.
        if (m_gsCollide.IsCoordInRect(m_gsMouse.GetX(), m_gsMouse.GetY(), rcOption))
        {
            // Remeber which option is highlighted.
            m_nOptionSelected = m_gsMenu.GetHighlight();
            m_bWasMouseReleased = FALSE;
            // Play the appropriate sound effect.
            m_gsSound.PlaySample(SAMPLE_SELECT);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Process Menu Commands ////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Act depending on option selected.
    switch (m_nOptionSelected)
    {
    case 0:
        // Set game progress.
        m_nNextProgress = PLAY_INTRO;
        m_nGameProgress = TITLE_OUTRO;
        // Reset selected option.
        m_nOptionSelected = -1;
        break;
    case 1:
        // Set game progress.
        m_nNextProgress = SCORES_INTRO;
        m_nGameProgress = TITLE_OUTRO;
        // Reset selected option.
        m_nOptionSelected = -1;
        break;
    case 2:
        // Set game progress.
        m_nNextProgress = OPTION_INTRO;
        m_nGameProgress = TITLE_OUTRO;
        // Reset selected option.
        m_nOptionSelected = -1;
        break;
    case 3:
        // Set game progress.
        m_nNextProgress = GAME_OUTRO;
        m_nGameProgress = TITLE_OUTRO;
        // Reset selected option.
        m_nOptionSelected = -1;
        break;
    default:
        break;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Logic //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Determine the interval required to perform an action 60 times every second at the
    // current frame rate, and add it to the previous intervals.
    m_fInterval += this->GetActionInterval(60);

    // Scroll the background image
    m_gsBackSprite.AddScrollX(this->GetActionInterval(60));

    // Have counter reached specified amount?
    if (m_fInterval >= 1200.0f)
    {
        // Set game progress.
        m_nNextProgress = SCORES_INTRO;
        m_nGameProgress = TITLE_OUTRO;
        m_fInterval = 0.0f;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Rendering //////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Clear the screen to the specified color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset display.
    glLoadIdentity();

    // Setup the display for rendering in 2D.
    m_gsDisplay.BeginRender2D(this->GetWindow());

    // Render the background image.
    this->RenderBackground();

    // Position the title centered in the top half of the screen.
    m_gsTitleSprite.SetDestX((INTERNAL_RES_X - m_gsTitleSprite.GetScaledWidth()) / 2);
    m_gsTitleSprite.SetDestY
    (
        (INTERNAL_RES_Y / 2) + (((INTERNAL_RES_Y / 2) - m_gsTitleSprite.GetScaledHeight()) / 2)
    );

    // Render the title images.
    // m_gsTitleSprite.Render();

    // Display the game logo using a scaled font
    m_gsFont.SetText("VINE HERO");
    m_gsFont.SetModulateColor(0.5f, 0.9f, 0.8f, 1.0f);
    m_gsFont.SetScaleXY(4.0f, 4.0f);
    m_gsFont.SetDestX((INTERNAL_RES_X / 2) - (m_gsFont.GetTextWidth() / 2));
    m_gsFont.SetDestY((INTERNAL_RES_Y / 1.5) - (m_gsFont.GetLetterHeight() / 2));
    m_gsFont.Render();

    // Display the version number at the bottom right of the screen.
    m_gsFont.SetModulateColor(-1.0f, -1.0f, -1.0f, 0.25f);
    m_gsFont.SetText("VERSION %s", VINE_HERO_VERSION);
    m_gsFont.SetScaleXY(0.5f, 0.5f);
    m_gsFont.SetDestX(INTERNAL_RES_X - m_gsFont.GetTextWidth() - m_gsFont.GetLetterWidth());
    m_gsFont.SetDestY(m_gsFont.GetTextHeight()) ;
    m_gsFont.Render();

    // Reset font scale and colors
    m_gsFont.SetModulateColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_gsFont.SetScaleXY(1.0f, 1.0f);

    // Position the menu centered in the bottom half of the screen.
    m_gsMenu.SetDestX((INTERNAL_RES_X - m_gsMenu.GetWidth()) / 2);
    m_gsMenu.SetDestY(((INTERNAL_RES_Y / 2) - m_gsMenu.GetHeight()) / 2);

    // Render the menu using the specified colors.
    m_gsMenu.Render(m_gsMenuTextColor, m_gsMenuHighColor);

    // Render the frame rate.
    this->RenderFrameRate(0.5f);

    // Render the mouse cursor.
    this->RenderCursor();

    // Reset display after rendering in 2D.
    m_gsDisplay.EndRender2D();

    // Swap buffers to display results.
    SwapBuffers(this->GetDevice());

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Cleanup ////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Are we about to leave this section?
    if (m_nGameProgress != TITLE_SCREEN)
    {
        // Remember where we've come from.
        m_nPrevProgress = TITLE_SCREEN;
        // Reset method variables.
        m_bIsInitialized = FALSE;
    }

    return TRUE;
}

//==============================================================================================
// GS_Demo::TitleOutro():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successfull and FALSE if not.
//==============================================================================================

BOOL GS_Demo::TitleOutro()
{

    // Has the display not been initialized?
    if (!this->m_gsDisplay.IsReady())
    {
        return FALSE;
    }

    // Has the game been paused?
    if (this->IsPaused())
    {
        // Unpause the game.
        this->Pause(FALSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Initialization /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Has the method not been initialized?
    if (m_bIsInitialized == FALSE)
    {
        // Initialize method variables.
        m_fAlpha    = 1.0f;
        // Initialization completed.
        m_bIsInitialized = TRUE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Logic //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Are images not entirely transparent?
    if (m_fAlpha > 0.0f)
    {
        // Increase the transparency.
        m_fAlpha -= this->GetActionInterval(60 * 0.025f);
    }
    else
    {
        // Progress to the next section.
        m_nGameProgress = m_nNextProgress;
        // Images are transparent.
        m_fAlpha = 0.0f;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Rendering //////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Clear the screen to the specified color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset display.
    glLoadIdentity();

    // Setup the display for rendering in 2D.
    m_gsDisplay.BeginRender2D(this->GetWindow());

    // Render the background image.
    this->RenderBackground(m_fAlpha);

    // Reset display after rendering in 2D.
    m_gsDisplay.EndRender2D();

    // Swap buffers to display results.
    SwapBuffers(this->GetDevice());

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Cleanup ////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Are we leaving this section?
    if (m_nGameProgress != TITLE_OUTRO)
    {
        // Remember where we've come from.
        m_nPrevProgress = TITLE_OUTRO;
        m_bIsInitialized = FALSE;
    }

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////
// OPTION SCREEN METHODS ///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


//==============================================================================================
// GS_Demo::OptionIntro():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successfull and FALSE if not.
//==============================================================================================

BOOL GS_Demo::OptionIntro()
{

    // Has the display not been initialized?
    if (!this->m_gsDisplay.IsReady())
    {
        return FALSE;
    }

    // Has the game been paused?
    if (this->IsPaused())
    {
        // Unpause the game.
        this->Pause(FALSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Initialization /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Has the method not been initialized?
    if (m_bIsInitialized == FALSE)
    {
        // Where we want to go to next.
        m_nNextProgress = OPTION_SCREEN;
        // Initialization completed.
        m_bIsInitialized = TRUE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Logic //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // ...

    // Progress to the next section.
    m_nGameProgress = m_nNextProgress;

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Rendering //////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Clear the screen to the specified color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset display.
    glLoadIdentity();

    // Setup the display for rendering in 2D.
    m_gsDisplay.BeginRender2D(this->GetWindow());

    // ...

    // Reset display after rendering in 2D.
    m_gsDisplay.EndRender2D();

    // Swap buffers to display results.
    SwapBuffers(this->GetDevice());

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Cleanup ////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Are we leaving this section?
    if (m_nGameProgress != OPTION_INTRO)
    {
        // Remember where we've come from.
        m_nPrevProgress = OPTION_INTRO;
        // Reset method variables.
        m_bIsInitialized = FALSE;
    }

    return TRUE;
}

//==============================================================================================
// GS_Demo::OptionScreen():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successfull and FALSE if not.
//==============================================================================================

BOOL GS_Demo::OptionScreen()
{

    // Has the display not been initialized?
    if (!this->m_gsDisplay.IsReady())
    {
        return FALSE;
    }

    // Has the game been paused?
    if (this->IsPaused())
    {
        // Unpause the game.
        this->Pause(FALSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Initialization /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    char cTempString[64] = { 0 };

    BOOL bLeftPressed = TRUE;

    // Has the method not been initialized?
    if (m_bIsInitialized == FALSE)
    {
        // Initialize method variables.
        m_nOptionSelected = -1;
        // Clear the keyboard buffer.
        m_gsKeyboard.ClearBuffer();
        // Setup variables for mouse input.
        m_nOldMouseX = m_gsMouse.GetX();
        m_nOldMouseY = m_gsMouse.GetY();
        m_bWasMouseReleased = FALSE;
        // Set the game settings to the current display settings.
        m_gsSettings.bEnableVSync  = m_gsDisplay.IsVSyncEnabled();
        m_gsSettings.bWindowedMode = this->IsWindowed();
        // Clear all current menu items.
        m_gsMenu.ClearOptions();
        // Set the menu title.
        m_gsMenu.SetTitle("   SELECT AN OPTION:  ");
        // Add menu options depending on settings.
        m_gsMenu.AddOption("");
        if (m_gsSettings.bWindowedMode)
        {
            m_gsMenu.AddOption("FULLSCREEN MODE.....NO");
        }
        else
        {
            m_gsMenu.AddOption("FULLSCREEN MODE....YES");
        }
        if (m_gsSettings.bEnableVSync)
        {
            m_gsMenu.AddOption("ENABLE VSYNC.......YES");
        }
        else
        {
            m_gsMenu.AddOption("ENABLE VSYNC........NO");
        }
        m_gsMenu.AddOption("");
        sprintf(cTempString, "MUSIC VOLUME.......%03d", m_gsSettings.nMusicVolume);
        m_gsMenu.AddOption(cTempString);
        sprintf(cTempString, "SOUND FX VOLUME....%03d", m_gsSettings.nEffectsVolume);
        m_gsMenu.AddOption(cTempString);
        m_gsMenu.AddOption("");
        m_gsMenu.AddOption("         DONE         ");
        m_gsMenu.AddOption("");
        // Highlight the first option.
        m_gsMenu.SetHighlight(1);
        // Where we want to go to next.
        m_nNextProgress = OPTION_OUTRO;
        // Initialization completed.
        m_bIsInitialized = TRUE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Get Keyboard Input ///////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Reset the selected option.
    m_nOptionSelected = -1;

    // Check to see wether a key was pressed.
    int nKey = m_gsKeyboard.GetBufferedKey();

    // Act depending on key pressed.
    switch (nKey)
    {
    // Was the up key pressed?
    case GSK_UP:
        // Highlight previous option.
        m_gsMenu.HighlightPrev();
        m_gsSound.PlaySample(SAMPLE_OPTION);
        break;
    // Was the down key pressed?
    case GSK_DOWN:
        // Highlight next option.
        m_gsMenu.HighlightNext();
        m_gsSound.PlaySample(SAMPLE_OPTION);
        break;
    // Was the left key pressed?
    case GSK_LEFT:
        // Is the last option not highlighted?
        if (m_gsMenu.GetHighlight() != 7)
        {
            // Remeber which option is highlighted.
            m_nOptionSelected = m_gsMenu.GetHighlight();
            m_gsSound.PlaySample(SAMPLE_SELECT);
            bLeftPressed = TRUE;
        }
        break;
    // Was the right key pressed?
    case GSK_RIGHT:
        // Is the last option not highlighted?
        if (m_gsMenu.GetHighlight() != 7)
        {
            // Remeber which option is highlighted.
            m_nOptionSelected = m_gsMenu.GetHighlight();
            m_gsSound.PlaySample(SAMPLE_SELECT);
            bLeftPressed = FALSE;
        }
        break;
    // Was the enter key pressed?
    case GSK_ENTER:
        // Remeber which option is highlighted.
        m_nOptionSelected = m_gsMenu.GetHighlight();
        m_gsSound.PlaySample(SAMPLE_SELECT);
        break;
    // Was the escape key pressed?
    case GSK_ESCAPE:
        // Progress to the next section.
        m_nGameProgress = m_nNextProgress;
        m_gsSound.PlaySample(SAMPLE_SELECT);
        break;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Get Mouse Input //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    RECT rcOption;

    // Has the mouse cursor moved from its previous position?
    if ((m_nOldMouseX != m_gsMouse.GetX()) || (m_nOldMouseY != m_gsMouse.GetY()))
    {
        // Check each option to see if the mouse coordinates are within the option area.
        for (int nLoop = 0; nLoop < m_gsMenu.GetNumOptions(); nLoop++)
        {
            // Determine the screen coordinates of the next option.
            m_gsMenu.GetOptionRect(nLoop, &rcOption);
            // Are the mouse coordinates within the option area?
            if (m_gsCollide.IsCoordInRect(m_gsMouse.GetX(), m_gsMouse.GetY(), rcOption))
            {
                // Is the option not highlighted already?
                if (nLoop != m_gsMenu.GetHighlight())
                {
                    // Highlight the selected option.
                    m_gsMenu.SetHighlight(nLoop);
                    // Play the appropriate sound effect.
                    m_gsSound.PlaySample(SAMPLE_OPTION);
                }
            }
        }
        // Assign new mouse coordinates.
        m_nOldMouseX = m_gsMouse.GetX();
        m_nOldMouseY = m_gsMouse.GetY();
    }

    // Is the left mouse button not pressed?
    if (!m_gsMouse.IsLeftPressed())
    {
        m_bWasMouseReleased = TRUE;
    }

    // Was the left mouse button pressed?
    if ((m_gsMouse.IsLeftPressed()) && (m_bWasMouseReleased))
    {
        // Determine the screen coordinates of the highlighted option.
        m_gsMenu.GetHighlightRect(&rcOption);
        // Check if mouse coordinates are within the area of the highligted option.
        if (m_gsCollide.IsCoordInRect(m_gsMouse.GetX(), m_gsMouse.GetY(), rcOption))
        {
            // Remeber which option is highlighted.
            m_nOptionSelected = m_gsMenu.GetHighlight();
            m_bWasMouseReleased = FALSE;
            // Play the appropriate sound effect.
            m_gsSound.PlaySample(SAMPLE_SELECT);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Process Menu Commands ////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Act depending on option selected.
    switch (m_nOptionSelected)
    {
    // Fullscreen or windowed mode?
    case 1:
        // Are we currently in windowed mode?
        if (m_gsSettings.bWindowedMode)
        {
            // Set menu text to reflect changes.
            m_gsMenu.SetOption(m_nOptionSelected, "FULLSCREEN MODE....YES");
            // Set to fullscreen mode.
            m_gsSettings.bWindowedMode = FALSE;
        }
        else
        {
            // Set menu text to reflect changes.
            m_gsMenu.SetOption(m_nOptionSelected, "FULLSCREEN MODE.....NO");
            // Set to windowed mode.
            m_gsSettings.bWindowedMode = TRUE;
        }
        break;
    // VSync enabled or disabled?
    case 2:
        // Is vertical sync currently enabled?
        if (m_gsSettings.bEnableVSync)
        {
            // Set menu text to reflect changes.
            m_gsMenu.SetOption(m_nOptionSelected, "ENABLE VSYNC........NO");
            // Disable vertical sync.
            m_gsSettings.bEnableVSync = FALSE;
        }
        else
        {
            // Set menu text to reflect changes.
            m_gsMenu.SetOption(m_nOptionSelected, "ENABLE VSYNC.......YES");
            // Enable vertical sync.
            m_gsSettings.bEnableVSync = TRUE;
        }
        // Enable / disable vertical sync depending on selection.
        m_gsDisplay.EnableVSync(m_gsSettings.bEnableVSync);
        break;
    // Determine music volume.
    case 4:
        // Was the left arrow pressed?
        if (bLeftPressed)
        {
            // Decrease music volume.
            m_gsSettings.nMusicVolume -= 15;
            // Wrap volume if out of bounds.
            if (m_gsSettings.nMusicVolume < 0)
            {
                m_gsSettings.nMusicVolume = 255;
            }
        }
        else
        {
            // Increase music volume.
            m_gsSettings.nMusicVolume += 15;
            // Wrap volume if out of bounds.
            if (m_gsSettings.nMusicVolume > 255)
            {
                m_gsSettings.nMusicVolume = 0;
            }
        }
        // Set music volume to reflect changes.
        m_gsSound.SetStreamVolume(m_gsSettings.nMusicVolume);
        // Set menu text to reflect changes.
        sprintf(cTempString, "MUSIC VOLUME.......%03d", m_gsSettings.nMusicVolume);
        m_gsMenu.SetOption(m_nOptionSelected, cTempString);
        break;
    // Determine sound effects volume.
    case 5:
        if (bLeftPressed)
        {
            // Decrease sound effects volume.
            m_gsSettings.nEffectsVolume -= 15;
            // Wrap volume if out of bounds.
            if (m_gsSettings.nEffectsVolume < 0)
            {
                m_gsSettings.nEffectsVolume = 255;
            }
        }
        else
        {
            // Increase sound effects volume.
            m_gsSettings.nEffectsVolume += 15;
            // Wrap volume if out of bounds.
            if (m_gsSettings.nEffectsVolume > 255)
            {
                m_gsSettings.nEffectsVolume = 0;
            }
        }
        // Set sound effects volume to reflect changes.
        m_gsSound.SetSampleMaster(m_gsSettings.nEffectsVolume);
        // Set menu text to reflect changes.
        sprintf(cTempString, "SOUND FX VOLUME....%03d", m_gsSettings.nEffectsVolume);
        m_gsMenu.SetOption(m_nOptionSelected, cTempString);
        break;
    // Done setting options.
    case 7:
        // Progress to the next section.
        m_nGameProgress = m_nNextProgress;
        break;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Rendering //////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Clear the screen to the specified color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset display.
    glLoadIdentity();

    // Setup the display for rendering in 2D.
    m_gsDisplay.BeginRender2D(this->GetWindow());

    // Render the background image.
    this->RenderBackground(0.5f);

    // Position the menu centered horizontally and vertically in the screen.
    m_gsMenu.SetDestX((INTERNAL_RES_X -  m_gsMenu.GetWidth()) / 2);
    m_gsMenu.SetDestY((INTERNAL_RES_Y - m_gsMenu.GetHeight()) / 2);

    // Render the menu using the specified colors.
    m_gsMenu.Render(m_gsMenuTextColor, m_gsMenuHighColor);

    // Render the frame rate.
    this->RenderFrameRate(0.5f);

    // Render the mouse cursor.
    this->RenderCursor();

    // Reset display after rendering in 2D.
    m_gsDisplay.EndRender2D();

    // Swap buffers to display results.
    SwapBuffers(this->GetDevice());

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Cleanup ////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Are we about to leave this section?
    if (m_nGameProgress != OPTION_SCREEN)
    {
        // Remember where we've come from.
        m_nPrevProgress = OPTION_SCREEN;

        // Have we changed the graphics mode?
        if (m_gsSettings.bWindowedMode != this->IsWindowed())
        {
            // Change to windowed or fullscreen mode as selected by the user.
            this->SetMode(m_gsSettings.nDisplayWidth, m_gsSettings.nDisplayHeight, this->GetColorDepth(), !this->IsWindowed());

            // Set render scaling values
            this->SetRenderScaling(this->GetWidth(), this->GetHeight(), true);

            // Has the mode change been successful?
            m_gsSettings.bWindowedMode = this->IsWindowed();
        }

        // Save game settings.
        this->SaveSettings();

        // Reset method variables.
        m_bIsInitialized  = FALSE;
    }

    return TRUE;
}

//==============================================================================================
// GS_Demo::OptionOutro():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successfull and FALSE if not.
//==============================================================================================

BOOL GS_Demo::OptionOutro()
{

    // Has the display not been initialized?
    if (!this->m_gsDisplay.IsReady())
    {
        return FALSE;
    }

    // Has the game been paused?
    if (this->IsPaused())
    {
        // Unpause the game.
        this->Pause(FALSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Initialization /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Has the method not been initialized?
    if (m_bIsInitialized == FALSE)
    {
        // Where we want to go to next.
        m_nNextProgress = TITLE_INTRO;
        // Initialization completed.
        m_bIsInitialized = TRUE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Logic //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // ...

    // Progress to the next section.
    m_nGameProgress = m_nNextProgress;

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Rendering //////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Clear the screen to the specified color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset display.
    glLoadIdentity();

    // Setup the display for rendering in 2D.
    m_gsDisplay.BeginRender2D(this->GetWindow());

    // ...

    // Reset display after rendering in 2D.
    m_gsDisplay.EndRender2D();

    // Swap buffers to display results.
    SwapBuffers(this->GetDevice());

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Cleanup ////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Are we leaving this section?
    if (m_nGameProgress != OPTION_OUTRO)
    {
        // Remember where we've come from.
        m_nPrevProgress = OPTION_OUTRO;
        // Reset method variables.
        m_bIsInitialized = FALSE;
    }

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////
// GAME PLAY METHODS ///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


//==============================================================================================
// GS_Demo::PlayIntro():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successfull and FALSE if not.
//==============================================================================================

BOOL GS_Demo::PlayIntro()
{

    // Has the display not been initialized?
    if (!this->m_gsDisplay.IsReady())
    {
        return FALSE;
    }

    // Has the game been paused?
    if (this->IsPaused())
    {
        // Unpause the game.
        this->Pause(FALSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Initialization /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Has the method not been initialized?
    if (m_bIsInitialized == FALSE)
    {
        // Initialize method variables.
        m_nOptionSelected = -1;
        // Clear the keyboard buffer.
        m_gsKeyboard.ClearBuffer();
        // Setup variables for mouse input.
        m_nOldMouseX = m_gsMouse.GetX();
        m_nOldMouseY = m_gsMouse.GetY();
        m_bWasMouseReleased = FALSE;
        // Re-create the hero as an unfiltered sprite for better display
        m_hero.Create(m_hero.GetSelectedHero(), m_hero.GetDestX(), m_hero.GetDestX(), false);
        m_hero.SetState(HERO_STATE_STANDING);
        // Clear all current menu items.
        m_gsHeroSelectMenu.ClearOptions();
        // Set the menu title.
        m_gsHeroSelectMenu.SetScaleXY(1.75f, 1.75f);
        m_gsHeroSelectMenu.SetTitle(" SELECT HERO: ");
        // Add the menu options.
        m_gsHeroSelectMenu.AddOption("");
        m_gsHeroSelectMenu.AddOption("");
        m_gsHeroSelectMenu.AddOption("#            $");
        m_gsHeroSelectMenu.AddOption("");
        m_gsHeroSelectMenu.AddOption("");
        // Highlight the second option.
        m_gsHeroSelectMenu.SetHighlight(2);
        // Clear all current menu items.
        m_gsMenu.ClearOptions();
        // Set the menu title.
        m_gsMenu.SetTitle(" SELECT DIFFICULTY: ");
        // Add the menu options.
        m_gsMenu.AddOption("");
        m_gsMenu.AddOption("        EASY        ");
        m_gsMenu.AddOption("       NORMAL       ");
        m_gsMenu.AddOption("        HARD        ");
        m_gsMenu.AddOption("");
        // Highlight the second option.
        m_gsMenu.SetHighlight(2);
        // Is the title music playing?
        if (m_gsSound.IsStreamPlaying())
        {
            // Stop title music from playing.
            m_gsSound.StopStream();
        }
        // Where we want to go to next.
        m_nNextProgress = PLAY_GAME;
        // Initialization completed.
        m_bIsInitialized = TRUE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Get Keyboard Input ///////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Check to see wether a key was pressed.
    int nKey = m_gsKeyboard.GetBufferedKey();

    // Act depending on key pressed.
    switch (nKey)
    {
    // Was the left key pressed?
    case GSK_LEFT:
        // Select a different hero
        if( m_hero.GetSelectedHero() == SELECT_HERO_JOHNNY )
        {
            // Re-create the hero
            m_hero.Create(SELECT_HERO_COURAGE, m_hero.GetDestX(), m_hero.GetDestX(), false);
            m_heroSampleIndex = m_hero.GetSelectedHero() * NUM_HERO_SAMPLES;
        }
        else
        {
            // Re-create the hero
            m_hero.Create(SELECT_HERO_JOHNNY, m_hero.GetDestX(), m_hero.GetDestX(), false);
            m_heroSampleIndex = m_hero.GetSelectedHero() * NUM_HERO_SAMPLES;
        }
        m_gsSound.PlaySample(SAMPLE_HERO_JUMP + m_heroSampleIndex);
        break;
    // Was the right key pressed?
    case GSK_RIGHT:
        // Select a different hero
        if( m_hero.GetSelectedHero() == SELECT_HERO_JOHNNY )
        {
            // Re-create the hero
            m_hero.Create(SELECT_HERO_COURAGE, m_hero.GetDestX(), m_hero.GetDestX(), false);
            m_heroSampleIndex = m_hero.GetSelectedHero() * NUM_HERO_SAMPLES;
        }
        else
        {
            // Re-create the hero
            m_hero.Create(SELECT_HERO_JOHNNY, m_hero.GetDestX(), m_hero.GetDestX(), false);
            m_heroSampleIndex = m_hero.GetSelectedHero() * NUM_HERO_SAMPLES;
        }
        m_gsSound.PlaySample(SAMPLE_HERO_JUMP + m_heroSampleIndex);
        break;
    // Was the up key pressed?
    case GSK_UP:
        // Highlight the previous option.
        m_gsMenu.HighlightPrev();
        m_gsSound.PlaySample(SAMPLE_OPTION);
        break;
    // Was the down key pressed?
    case GSK_DOWN:
        // Highlight the next option.
        m_gsMenu.HighlightNext();
        m_gsSound.PlaySample(SAMPLE_OPTION);
        break;
    // Was the enter key pressed?
    case GSK_ENTER:
        // Remeber which option is highlighted.
        m_nOptionSelected = m_gsMenu.GetHighlight();
        m_gsSound.PlaySample(SAMPLE_SELECT);
        break;
    // Was the escape key pressed?
    case GSK_ESCAPE:
        // Return to the title screen.
        m_nGameProgress = TITLE_INTRO;
        m_bWasKeyReleased = FALSE;
        break;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Get Mouse Input //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    RECT rcOption;

    // Has the mouse cursor moved from its previous position?
    if ((m_nOldMouseX != m_gsMouse.GetX()) || (m_nOldMouseY != m_gsMouse.GetY()))
    {
        // Check each option to see if the mouse coordinates are within the option area.
        for (int nLoop = 0; nLoop < m_gsMenu.GetNumOptions(); nLoop++)
        {
            // Determine the screen coordinates of the next option.
            m_gsMenu.GetOptionRect(nLoop, &rcOption);

            // Are the mouse coordinates within the option area?
            if (m_gsCollide.IsCoordInRect(m_gsMouse.GetX(), m_gsMouse.GetY(), rcOption))
            {
                // Is the option not highlighted already?
                if (nLoop != m_gsMenu.GetHighlight())
                {
                    // Highlight the selected option.
                    m_gsMenu.SetHighlight(nLoop);

                    // Play the appropriate sound effect.
                    m_gsSound.PlaySample(SAMPLE_OPTION);
                }
            }
        }

        // Assign new mouse coordinates.
        m_nOldMouseX = m_gsMouse.GetX();
        m_nOldMouseY = m_gsMouse.GetY();
    }

    // Is the left mouse button not pressed?
    if (!m_gsMouse.IsLeftPressed())
    {
        m_bWasMouseReleased = TRUE;
    }

    // Was the left mouse button pressed?
    if ((m_gsMouse.IsLeftPressed()) && (m_bWasMouseReleased))
    {
        // Determine the screen coordinates of the highlighted option.
        m_gsMenu.GetHighlightRect(&rcOption);

        // Check if mouse coordinates are within the area of the highligted option.
        if (m_gsCollide.IsCoordInRect(m_gsMouse.GetX(), m_gsMouse.GetY(), rcOption))
        {
            // Remeber which option is highlighted.
            m_nOptionSelected = m_gsMenu.GetHighlight();
            m_bWasMouseReleased = FALSE;

            // Play the appropriate sound effect.
            m_gsSound.PlaySample(SAMPLE_SELECT);
        }

        // Determine the screen coordinates of the highlighted option.
        m_gsHeroSelectMenu.GetHighlightRect(&rcOption);

        // Check if the button on the left was clicked
        long oldOptionRight = rcOption.right;
        rcOption.right = rcOption.left + m_gsFont.GetLetterWidth() * m_gsHeroSelectMenu.GetScaleX();

        // Check if mouse coordinates are within the area of the highligted option.
        if (m_gsCollide.IsCoordInRect(m_gsMouse.GetX(), m_gsMouse.GetY(), rcOption))
        {
            // Select a different hero
            if( m_hero.GetSelectedHero() == SELECT_HERO_JOHNNY )
            {
                // Re-create the hero
                m_hero.Create(SELECT_HERO_COURAGE, m_hero.GetDestX(), m_hero.GetDestX(), false);
                m_heroSampleIndex = m_hero.GetSelectedHero() * NUM_HERO_SAMPLES;
            }
            else
            {
                // Re-create the hero
                m_hero.Create(SELECT_HERO_JOHNNY, m_hero.GetDestX(), m_hero.GetDestX(), false);
                m_heroSampleIndex = m_hero.GetSelectedHero() * NUM_HERO_SAMPLES;
            }

            // Play the appropriate sample
            m_gsSound.PlaySample(SAMPLE_HERO_JUMP + m_heroSampleIndex);

            // Wait for mouse button to be released
            m_bWasMouseReleased = FALSE;
        }
        else
        {
            // Check if the button on the left was clicked
            rcOption.right = oldOptionRight;
            rcOption.left = rcOption.right - m_gsFont.GetLetterWidth() * m_gsHeroSelectMenu.GetScaleX();

            // Check if mouse coordinates are within the area of the highligted option.
            if (m_gsCollide.IsCoordInRect(m_gsMouse.GetX(), m_gsMouse.GetY(), rcOption))
            {
                // Select a different hero
                if( m_hero.GetSelectedHero() == SELECT_HERO_JOHNNY )
                {
                    // Re-create the hero
                    m_hero.Create(SELECT_HERO_COURAGE, 0, 0, false);
                    m_heroSampleIndex = m_hero.GetSelectedHero() * NUM_HERO_SAMPLES;
                }
                else
                {
                    // Re-create the hero
                    m_hero.Create(SELECT_HERO_JOHNNY, 0, 0, false);
                    m_heroSampleIndex = m_hero.GetSelectedHero() * NUM_HERO_SAMPLES;
                }

                // Play the appropriate sample
                m_gsSound.PlaySample(SAMPLE_HERO_JUMP + m_heroSampleIndex);

                // Wait for mouse button to be released
                m_bWasMouseReleased = FALSE;
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Process Menu Commands ////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Act depending on option selected.
    switch (m_nOptionSelected)
    {
    // ...
    case 1:
        // Set game mode
        m_nGameMode = GAME_MODE_EASY;

        // Set play values
        m_nMaxVineElements = MAX_VINE_ELEMENTS;
        m_nMinVineElements = MIN_VINE_ELEMENTS + ((MAX_VINE_ELEMENTS - MIN_VINE_ELEMENTS) / 2);
        m_nMaxVineSpeed = MAX_VINE_SPEED;
        m_nMinVineSpeed = MIN_VINE_SPEED;
        m_nMaxHeroSwings = MAX_HERO_SWINGS * 2;

        // Progress to the next section.
        m_nGameProgress = m_nNextProgress;
        break;
    // ...
    case 2:
        // Set game mode
        m_nGameMode = GAME_MODE_NORMAL;

        m_nMaxVineElements = MAX_VINE_ELEMENTS;
        m_nMinVineElements = MIN_VINE_ELEMENTS;
        m_nMaxVineSpeed = MAX_VINE_SPEED;
        m_nMinVineSpeed = MIN_VINE_SPEED;
        m_nMaxHeroSwings = MAX_HERO_SWINGS;

        // Progress to the next section.
        m_nGameProgress = m_nNextProgress;
        break;
    // ...
    case 3:
        // Set game mode
        m_nGameMode = GAME_MODE_HARD;

        m_nMaxVineElements = MIN_VINE_ELEMENTS + ((MAX_VINE_ELEMENTS - MIN_VINE_ELEMENTS) / 2);
        m_nMinVineElements = MIN_VINE_ELEMENTS;
        m_nMaxVineSpeed = MAX_VINE_SPEED;
        m_nMinVineSpeed = MIN_VINE_SPEED;
        m_nMaxHeroSwings = MAX_HERO_SWINGS / 2;

        // Progress to the next section.
        m_nGameProgress = m_nNextProgress;
        break;
    // ...
    default:
        break;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Rendering //////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Clear the screen to the specified color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset display.
    glLoadIdentity();

    // Setup the display for rendering in 2D.
    m_gsDisplay.BeginRender2D(this->GetWindow());

    // Center the hero select menu horizontally and in the top two thirds of the screen
    m_gsHeroSelectMenu.SetDestX((INTERNAL_RES_X - m_gsHeroSelectMenu.GetWidth())  / 2);
    m_gsHeroSelectMenu.SetDestY
    (
        INTERNAL_RES_Y / 3 + ((INTERNAL_RES_Y * (2.0f/3.0f)) - m_gsHeroSelectMenu.GetHeight()) / 2
    );

    // Render the menu to the back surface using the specified colors.
    m_gsHeroSelectMenu.Render(m_gsMenuTextColor, m_gsMenuHighColor);

    // Display the selected hero centered in the menu (minus the menu title)
    RECT optionRect;
    m_gsHeroSelectMenu.GetOptionRect(0, &optionRect);
    m_hero.SetPosition
    (
        m_gsHeroSelectMenu.GetDestX() + ((m_gsHeroSelectMenu.GetWidth() - m_hero.GetWidth()) / 2),
        m_gsHeroSelectMenu.GetDestY() + ((m_gsHeroSelectMenu.GetHeight() - ((optionRect.top - optionRect.bottom) * 2) - m_hero.GetHeight() ) / 2)
    );
    m_hero.Render();

    // Center the menu horizontally and vertically underneath the hero select menu
    m_gsMenu.SetDestX((INTERNAL_RES_X - m_gsMenu.GetWidth())  / 2);
    m_gsMenu.SetDestY((m_gsHeroSelectMenu.GetDestY() - m_gsMenu.GetHeight()) / 2);

    // Render the menu to the back surface using the specified colors.
    m_gsMenu.Render(m_gsMenuTextColor, m_gsMenuHighColor);

    // Render the mouse cursor.
    this->RenderCursor();

    // Reset display after rendering in 2D.
    m_gsDisplay.EndRender2D();

    // Swap buffers to display results.
    SwapBuffers(this->GetDevice());

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Cleanup ////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Are we leaving this section?
    if (m_nGameProgress != PLAY_INTRO)
    {
        // Remember where we've come from.
        m_nPrevProgress = PLAY_INTRO;
        // Reset method variables.
        m_bIsInitialized  = FALSE;
        // Re-create the hero as a filtered sprite for better rotation and scaling
        m_hero.Create(m_hero.GetSelectedHero(), m_hero.GetDestX(), m_hero.GetDestX(), true);
        // Are we about to start playing the game?
        if (m_nGameProgress == PLAY_GAME)
        {
            // Start playing game music at the specified volume, 1x speed and looping infinitely.
            m_gsSound.PlayStream(MUSIC_GAME, m_gsSettings.nMusicVolume);
        }
    }


    return TRUE;
}

//==============================================================================================
// GS_Demo::PlayGame():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successfull and FALSE if not.
//==============================================================================================

BOOL GS_Demo::PlayGame()
{
    int keyList[1] = { GSK_SPACE };

    // Has the display not been initialized?
    if (!this->m_gsDisplay.IsReady())
    {
        return FALSE;
    }

    // Has the game been paused?
    if (this->IsPaused())
    {
        // Remember where we've come from.
        m_nPrevProgress = m_nGameProgress;
        // Progress to pause section.
        m_nGameProgress = PLAY_PAUSE;
        return TRUE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Initialization /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Are we starting a new game?
    if (m_nPrevProgress == PLAY_INTRO)
    {
        // Reset class variables.
        m_nPrevProgress = PLAY_GAME;
        m_fInterval = 0.0f;
        m_nCounter  = 0;
        m_lScore = 0;

        // Reset vine swing speed
        m_vineLastSwingSpeed = rand()%(m_nMaxVineSpeed-m_nMinVineSpeed) + m_nMinVineSpeed;

        // Create the first vine at the edge of the screen
        m_vines[0].Create
        (
            rand()%(m_nMaxVineElements-m_nMinVineElements) + m_nMinVineElements, // number of elements
            0, // start x
            INTERNAL_RES_Y - 10,  // start y
            m_vineLastSwingSpeed, // swing speed
            -1, // max amplitude of swing
            -1, // min amplitude of swing
            -1  // swing width
        );

        float lastVinePosX = m_vines[0].GetDestX();

        // Create all additional vines
        for( int nLoop = 1; nLoop < MAX_VINES; nLoop++)
        {
            // Make certain the swing speed for the next vine is different
            int newSwingSpeed = rand()%(MAX_VINE_SPEED-MIN_VINE_SPEED) + MIN_VINE_SPEED;
            while( (newSwingSpeed > m_vineLastSwingSpeed - 5) && (newSwingSpeed < m_vineLastSwingSpeed + 5) )
            {
                newSwingSpeed = rand()%(MAX_VINE_SPEED-MIN_VINE_SPEED) + MIN_VINE_SPEED;
            }
            m_vineLastSwingSpeed = newSwingSpeed;

            // Create the vine
            m_vines[nLoop].Create
            (
                rand()%(MAX_VINE_ELEMENTS-MIN_VINE_ELEMENTS) + MIN_VINE_ELEMENTS, // number of elements
                lastVinePosX + float((float)INTERNAL_RES_X / (float)MAX_VINES),   // start x
                INTERNAL_RES_Y - 10,            // start y
                m_vineLastSwingSpeed,           // swing speed
                -1,                             // max amplitude of swing
                -1,                             // min amplitude of swing
                -1                              // swing width
            );
            lastVinePosX += float((float)INTERNAL_RES_X / (float)MAX_VINES);
        }

        // Set default values
        m_vineIndex = MAX_VINES - 1;
        m_vineElementIndex = m_vines[m_vineIndex].GetNumElements() - 3;
        m_vineSwingSamplePlayed = false;
        m_heroSwingCount = 0;

        // Update hero state and position
        m_hero.SetPosition
        (
            m_vines[m_vineIndex].GetElementDestX( m_vineElementIndex ) + 5,
            m_vines[m_vineIndex].GetElementDestY( m_vineElementIndex ) - 50
        );
        m_hero.SetState(HERO_STATE_SWINGING);

        m_fInterval = 0.0f;

        // Clear the keyboard buffer.
        m_gsKeyboard.ClearBuffer();
    }
    // Are we returning to the same game?
    else if (m_nPrevProgress != PLAY_GAME)
    {
        // Reset class variables.
        m_nPrevProgress = PLAY_GAME;
        // Clear the keyboard buffer.
        m_gsKeyboard.ClearBuffer();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Get Mouse Input //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Is the left mouse button not pressed?
    if (!m_gsMouse.IsLeftPressed())
    {
        m_bWasMouseReleased = TRUE;
    }

    // Was the left mouse button pressed?
    if ((m_gsMouse.IsLeftPressed()) && (m_bWasMouseReleased))
    {
        // Hero jumps when left mouse button is pressed
        if( m_hero.GetState() == HERO_STATE_SWINGING )
        {
            m_hero.SetState(HERO_STATE_JUMPING);
            m_gsSound.PlaySample(SAMPLE_HERO_JUMP + m_heroSampleIndex);
            m_bWasMouseReleased = FALSE;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Get Keyboard Input ///////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Were all the keys in the key list released?
    if (TRUE == m_gsKeyboard.AreKeysUp(1, keyList))
    {
        // Set flag to indicate that all the keys were released.
        m_bWasKeyReleased = TRUE;
    }

    // Check to see wether a key was pressed.
    int nKey = m_gsKeyboard.GetBufferedKey();

    // Clear the keyboard buffer.
    m_gsKeyboard.ClearBuffer();

    // Act depending on which key was pressed.
    switch (nKey)
    {
    // Was left arrow key pressed?
    case GSK_LEFT:
        // Make the vine swing slower
        m_vines[m_vineIndex].SetSwingSpeed( m_vines[m_vineIndex].GetSwingSpeed() - 1 );
        m_lScore = 0;
        break;
    // Was right arrow key pressed?
    case GSK_RIGHT:
        // Make the vine swing faster
        m_vines[m_vineIndex].SetSwingSpeed( m_vines[m_vineIndex].GetSwingSpeed() + 1 );
        m_lScore = 0;
        break;
    // Was down arrow key pressed?
    case GSK_DOWN:
        // Move the hero down on the vine, if possible
        if( m_vineElementIndex < m_vines[m_vineIndex].GetNumElements() - 1 ) m_vineElementIndex++;
        m_lScore = 0;
        break;
    // Was up arrow key pressed?
    case GSK_UP:
        // Move the hero up on the vine, if possible
        if( m_vineElementIndex > 0 ) m_vineElementIndex--;
        m_lScore = 0;
        break;
    // Was the 'M' key pressed?
    case GSK_M:
        // Is the music volume muted?
        if (m_gsSound.GetStreamVolume() == 0)
        {
            // Set the music volume to the game settings volume.
            m_gsSound.SetStreamVolume(m_gsSettings.nMusicVolume);
        }
        else
        {
            // Mute the music volume.
            m_gsSound.SetStreamVolume(0);
        }
        break;
    // Was the 'S' key pressed?
    case GSK_S:
        // Is the sound effects volume muted?
        if (m_gsSound.GetSampleMaster() == 0)
        {
            // Set the sound effects volume to the game settings volume.
            m_gsSound.SetSampleMaster(m_gsSettings.nEffectsVolume);
        }
        else
        {
            // Mute the sound effects volume.
            m_gsSound.SetSampleMaster(0);
        }
        break;
    // Was the spacebar key pressed?
    case GSK_SPACE:
        // Was this key released?
        if (m_bWasKeyReleased)
        {
            // Hero jumps when space bar is pressed
            if( m_hero.GetState() == HERO_STATE_SWINGING )
            {
                m_hero.SetState(HERO_STATE_JUMPING);
                m_gsSound.PlaySample(SAMPLE_HERO_JUMP + m_heroSampleIndex);
                m_bWasKeyReleased = FALSE;
            }
        }
        break;
    // Was the escape key pressed?
    case GSK_ESCAPE:
        // Go to the play exit method.
        m_nGameProgress = PLAY_EXIT;
        break;
    default:
        // ...
        break;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Logic //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    int scrollPoint = INTERNAL_RES_X / 1.5;
    int x, y, vineElement, scrollX;
    RECT heroRect;

    // Determine the interval required to perform an action 60 times every second at the
    // current frame rate, and add it to the previous intervals.
    m_fInterval += this->GetActionInterval(60);

    // Have all the intervals added up enough to perform an action?
    while (m_fInterval >= 1.0f)
    {
        // Get the collision area for the hero
        m_hero.GetCollideRect(&heroRect);

        // For every vine
        for (int nLoop = 0; nLoop < MAX_VINES; nLoop++)
        {
            // Update vine movement
            m_vines[nLoop].UpdateSwing();

            // Is the hero swinging on this vine?
            if((m_vineIndex == nLoop) && (m_hero.GetState() == HERO_STATE_SWINGING))
            {
                RECT vineStartElementRect;
                RECT vineLastElementRect;

                // Get the coordinates of the first and last element of the vine
                m_vines[nLoop].GetElementRect(0, &vineStartElementRect);
                m_vines[nLoop].GetElementRect(m_vines[nLoop].GetNumElements() - 1, &vineLastElementRect);

                // Adjust the y coordinates of the first element to match the last element
                vineStartElementRect.top = vineLastElementRect.top;
                vineStartElementRect.bottom = vineLastElementRect.bottom;

                // Is there a collision between the adjusted rectangle and the last element?
                if( m_gsCollide.IsRectOnRect(vineStartElementRect, vineLastElementRect, 100) )
                {
                    // Has the vine swing sample not been played?
                    if( !m_vineSwingSamplePlayed )
                    {
                        // Play the vine swing sample and remember that it's been played
                        m_gsSound.PlaySample(SAMPLE_VINE_SWOOSH);
                        m_vineSwingSamplePlayed = true;

                        // One more swing
                        m_heroSwingCount++;

                        // Has the hero swinged more than the allowed times?
                        if( m_heroSwingCount > m_nMaxHeroSwings )
                        {
                            // Is the hero not at the bottom of the vine?
                            // if( m_vineElementIndex < m_vines[m_vineIndex].GetNumElements() - 1 )
                            // {
                                // Slip one vine element down
                                // m_vineElementIndex++;
                            // }
                            // else
                            // {
                                // Fall of the vine
                                m_hero.SetState(HERO_STATE_FALLING);
                            // }
                        }
                    }

                }
                else
                {
                    // The vine swing sample can be played again
                    m_vineSwingSamplePlayed = false;
                }
            }

            // Is it NOT the vine the hero is currently swinging on and is the hero jumping?
            if( (m_vineIndex != nLoop) && (m_hero.GetState() == HERO_STATE_JUMPING) )
            {
                // Check for collision between the hero and any of the elements in the new vine
                vineElement = m_vines[nLoop].GetCollideElement(heroRect);

                // Was there a collision?
                if( vineElement != -1)
                {
                    // Remember the vine and element the hero is swinging on
                    m_vineElementIndex = vineElement;
                    m_vineIndex = nLoop;

                    // Set the hero state to swinging
                    m_hero.SetState(HERO_STATE_SWINGING);
                    m_heroSwingCount = 0;

                    // Add to the player's score
                    if( m_nGameMode == GAME_MODE_EASY)
                    {
                        m_lScore += 50;
                    }
                    else if( m_nGameMode == GAME_MODE_NORMAL)
                    {
                        m_lScore += 100;
                    }
                    else if( m_nGameMode == GAME_MODE_HARD)
                    {
                        m_lScore += 150;
                    }
                }
            }
        }

        // Is the hero swinging?
        if( HERO_STATE_SWINGING == m_hero.GetState() )
        {
            // Update the hero coordinates according the the element of the vine he is swinging on
            x = m_vines[m_vineIndex].GetElementDestX( m_vineElementIndex );
            y = m_vines[m_vineIndex].GetElementDestY( m_vineElementIndex );
            m_hero.SetPosition(x, y - 100);

            // Update the hero swing movement setting the rotation based on the angle between
            // the first element of the vine and the element the hero is swining on
            float rotateZ = atan2
                (
                    m_vines[m_vineIndex].GetElementDestY(0) - y,
                    m_vines[m_vineIndex].GetElementDestX(0) - x
                ) * 100.0f - 180.0f;

            m_hero.UpdateMovement((m_vineElementIndex < 3) ? 0.0f : rotateZ / 2.0f);
        }
        // Is the hero jumping or falling?
        else if( (HERO_STATE_JUMPING == m_hero.GetState()) ||
                 (HERO_STATE_FALLING == m_hero.GetState()) )
        {
            // Update the hero jump or fall movement
            m_hero.UpdateMovement();
        }

        // Has the hero died?
        if( m_hero.GetDestY() < 0 - m_hero.GetHeight() )
        {
            m_gsSound.PlaySample(SAMPLE_HERO_DEATH + m_heroSampleIndex);
            m_nGameProgress = PLAY_OUTRO;
        }

        // Has the hero moved past the scrolling point and is the hero swinging?
        if( (m_hero.GetDestX() < scrollPoint) && (m_hero.GetDestY() > 0) &&
            (m_hero.GetState() == HERO_STATE_SWINGING) )
        {
            // Adjust the scrolling of the background image
            scrollX = ((scrollPoint - m_hero.GetDestX()) / 10.f);
            m_gsBackSprite.AddScrollX(scrollX);

            // For every vine
            for (int nLoop = 0; nLoop < MAX_VINES; nLoop++)
            {
                // Update vine movement
                m_vines[nLoop].AddDestX(scrollX);

                // Has the vine left the screen?
                if( m_vines[nLoop].GetDestX() > INTERNAL_RES_X )
                {
                    // Make certain the swing speed for the next vine is different
                    int newSwingSpeed = rand()%(m_nMaxVineSpeed-m_nMinVineSpeed) + m_nMinVineSpeed;
                    while( (newSwingSpeed > m_vineLastSwingSpeed - 5) && (newSwingSpeed < m_vineLastSwingSpeed + 5) )
                    {
                        newSwingSpeed = rand()%(m_nMaxVineSpeed-m_nMinVineSpeed) + m_nMinVineSpeed;
                    }
                    m_vineLastSwingSpeed = newSwingSpeed;

                    // Create a new vine to swing to
                    m_vines[nLoop].Create
                    (
                        rand()%(m_nMaxVineElements-m_nMinVineElements) + m_nMinVineElements, // number of elements
                        m_vines[nLoop].GetDestX() - INTERNAL_RES_X, // start x
                        INTERNAL_RES_Y - 10,  // start y
                        m_vineLastSwingSpeed, // swing speed
                        -1, // max amplitude of swing
                        -1, // min amplitude of swing
                        -1  // swing width
                    );
                }
            }
        }

        // One action less to be taken.
        m_fInterval -= 1.0f;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Rendering //////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Clear the screen to the specified color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset display.
    glLoadIdentity();

    // Setup the display for rendering in 2D.
    m_gsDisplay.BeginRender2D(this->GetWindow());

    // Render the background image.
    this->RenderBackground();

    // Display the player score.
    m_gsFont.SetText("%ld", m_lScore);
    m_gsFont.SetModulateColor(0.5f, 1.0f, 0.8f, 1.0f);
    m_gsFont.SetScaleXY(2.0f, 2.0f);
    m_gsFont.SetDestX((INTERNAL_RES_X / 2)  - (m_gsFont.GetTextWidth() / 2));
    m_gsFont.SetDestY(m_gsFont.GetLetterHeight() / 2);
    m_gsFont.Render();

    // Is the hero within three swings of slipping or falling?
    if( m_heroSwingCount >=  m_nMaxHeroSwings - 3 )
    {
        // Has the hero not yet reached the maximum allowed swings?
        if( m_heroSwingCount < m_nMaxHeroSwings )
        {
            // Display the number of swings left
            m_gsFont.SetText("%d", m_nMaxHeroSwings - m_heroSwingCount );
            m_gsFont.SetScaleXY(3.0f, 3.0f);
        }
        else
        {
            // Has the hero not reached the bottom of the vine?
            // if( m_vineElementIndex < m_vines[m_vineIndex].GetNumElements() - 1 )
            // {
                // Display a message that the hero is slipping
                // m_gsFont.SetText("HURRY!");
                // m_gsFont.SetScaleXY(3.0f, 3.0f);

            // }
            // else
            // {
                // Display a message to jump
                m_gsFont.SetText("JUMP!");
                m_gsFont.SetScaleXY(4.0f, 4.0f);
            // }
        }
        // Display a large red message centered in the screen in red
        m_gsFont.SetModulateColor(1.0f, 0.5f, 0.5f, 0.5f);
        m_gsFont.SetDestX((INTERNAL_RES_X -  m_gsFont.GetTextWidth()) / 2);
        m_gsFont.SetDestY((INTERNAL_RES_Y - m_gsFont.GetTextHeight()) / 2);
        m_gsFont.Render();
    }

    // Display the hero
    m_hero.Render();

    // Display the vines
    for (int nLoop = 0; nLoop < MAX_VINES; nLoop++)
    {
        m_vines[nLoop].Render();
    }

    // Reset the default font values
    m_gsFont.SetModulateColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_gsFont.SetScaleXY(1.0f, 1.0f);

    // Render the frame rate.
    this->RenderFrameRate(0.5f);

    // Reset display after rendering in 2D.
    m_gsDisplay.EndRender2D();

    // Swap buffers to display results.
    SwapBuffers(this->GetDevice());

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Cleanup ////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Are we about to leave the game?
    if (m_nGameProgress != PLAY_GAME)
    {
        // Remember where we've come from.
        m_nPrevProgress = PLAY_GAME;
    }

    return TRUE;
}

//==============================================================================================
// GS_Demo::PlayUpdate():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successfull and FALSE if not.
//==============================================================================================

BOOL GS_Demo::PlayUpdate()
{

    // Has the display not been initialized?
    if (!this->m_gsDisplay.IsReady())
    {
        return FALSE;
    }

    // Has the game been paused?
    if (this->IsPaused())
    {
        // Remember where we've come from.
        m_nPrevProgress = m_nGameProgress;
        // Progress to pause section.
        m_nGameProgress = PLAY_PAUSE;
        return TRUE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Initialization /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Has the method not been initialized?
    if (m_bIsInitialized == FALSE)
    {
        // ...
        // Initialization completed.
        m_bIsInitialized = TRUE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Logic //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // ...

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Rendering //////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Clear the screen to the specified color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset display.
    glLoadIdentity();

    // Setup the display for rendering in 2D.
    m_gsDisplay.BeginRender2D(this->GetWindow());

    // ...

    // Reset display after rendering in 2D.
    m_gsDisplay.EndRender2D();

    // Swap buffers to display results.
    SwapBuffers(this->GetDevice());

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Cleanup ////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Are we leaving this section?
    if (m_nGameProgress != PLAY_UPDATE)
    {
        // Remember where we've come from.
        m_nPrevProgress = PLAY_UPDATE;
        // Reset method variables.
        m_bIsInitialized = FALSE;
    }

    return TRUE;
}

//==============================================================================================
// GS_Demo::PlayPause():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successfull and FALSE if not.
//==============================================================================================

BOOL GS_Demo::PlayPause()
{

    // Has the display not been initialized?
    if (!this->m_gsDisplay.IsReady())
    {
        return FALSE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Initialization /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Has the method not been initialized?
    if (m_bIsInitialized == FALSE)
    {
        // Play the appropriate sound effect.
        m_gsSound.PlaySample(SAMPLE_OPTION);
        // Initialization completed.
        m_bIsInitialized = TRUE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Logic //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Is the game no longer paused?
    if (!this->IsPaused())
    {
        // Return to the section we came from.
        m_nGameProgress = m_nPrevProgress;
        // Remember where we've come from.
        m_nPrevProgress = PLAY_PAUSE;
        // Play the appropriate sound effect.
        m_gsSound.PlaySample(SAMPLE_SELECT);
        // Reset method variables.
        m_bIsInitialized = FALSE;
        return TRUE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Rendering //////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Clear the screen to the specified color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset display.
    glLoadIdentity();

    // Setup the display for rendering in 2D.
    m_gsDisplay.BeginRender2D(this->GetWindow());

    // Render the background image.
    this->RenderBackground(0.5f);

    // Display the hero
    m_hero.Render(0.5f);

    // Display the vines
    for (int nLoop = 0; nLoop < MAX_VINES; nLoop++)
    {
        m_vines[nLoop].Render(0.5f);
    }

    // Render the frame rate.
    this->RenderFrameRate(0.5f);

    // Display a message centered in the screen.
    m_gsFont.SetText("GAME PAUSED");
    m_gsFont.SetDestX((INTERNAL_RES_X -  m_gsFont.GetTextWidth()) / 2);
    m_gsFont.SetDestY(((INTERNAL_RES_Y - m_gsFont.GetTextHeight()) / 2) +
                      m_gsFont.GetTextHeight());
    m_gsFont.Render();

    m_gsFont.SetText("(PRESS P)");
    m_gsFont.SetDestX((INTERNAL_RES_X -  m_gsFont.GetTextWidth()) / 2);
    m_gsFont.SetDestY(((INTERNAL_RES_Y - m_gsFont.GetTextHeight()) / 2) -
                      m_gsFont.GetTextHeight());
    m_gsFont.Render();

    // Reset display after rendering in 2D.
    m_gsDisplay.EndRender2D();

    // Swap buffers to display results.
    SwapBuffers(this->GetDevice());

    return TRUE;
}

//==============================================================================================
// GS_Demo::PlayExit():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successfull and FALSE if not.
//==============================================================================================

BOOL GS_Demo::PlayExit()
{
    int keyList[6] = { GSK_ENTER, GSK_UP, GSK_DOWN, GSK_Y, GSK_N, GSK_ESCAPE };

    // Has the display not been initialized?
    if (!this->m_gsDisplay.IsReady())
    {
        return FALSE;
    }

    // Has the game been paused?
    if (this->IsPaused())
    {
        // Unpause the game.
        this->Pause(FALSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Initialization /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Has the method not been initialized?
    if (m_bIsInitialized == FALSE)
    {
        // Initialize method variables.
        m_bWasKeyReleased = FALSE;
        // Clear keyboard buffer.
        m_gsKeyboard.ClearBuffer();
        // Setup variables for mouse input.
        m_nOldMouseX = m_gsMouse.GetX();
        m_nOldMouseY = m_gsMouse.GetY();
        m_bWasMouseReleased = FALSE;
        // Play the appropriate sound effect.
        m_gsSound.PlaySample(SAMPLE_OPTION);
        // Clear all current menu items.
        m_gsMenu.ClearOptions();
        // Set menu title.
        m_gsMenu.SetTitle("  END GAME?  ");
        // Highlight the first option.
        m_gsMenu.SetHighlight(0);
        // Initialization completed.
        m_bIsInitialized = TRUE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Set Menu Options /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Clear all current menu items.
    m_gsMenu.ClearOptions();

    // Add menu options depending on settings.
    m_gsMenu.AddOption("     YUP     ");
    m_gsMenu.AddOption("     NAH     ");

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Get Keyboard Input ///////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Reset the selected option.
    m_nOptionSelected = -1;

    // Were all the keys in the key list released?
    if (TRUE == m_gsKeyboard.AreKeysUp(6, keyList))
    {
        m_bWasKeyReleased = TRUE;
    }

    // Check to see wether a key was pressed.
    int nKey = m_gsKeyboard.GetKeyPressed();

    // Act depending on key pressed.
    switch (nKey)
    {
    // Was the up key pressed?
    case GSK_UP:
        if (m_bWasKeyReleased)
        {
            // Highlight previous option.
            m_gsMenu.HighlightPrev();
            m_gsSound.PlaySample(SAMPLE_OPTION);
            m_bWasKeyReleased = FALSE;
        }
        break;
    // Was the down key pressed?
    case GSK_DOWN:
        if (m_bWasKeyReleased)
        {
            // Highlight next option.
            m_gsMenu.HighlightNext();
            m_gsSound.PlaySample(SAMPLE_OPTION);
            m_bWasKeyReleased = FALSE;
        }
        break;
    // Was the up key pressed?
    case GSK_Y:
        if (m_bWasKeyReleased)
        {
            // Highlight yes option.
            m_gsMenu.SetHighlight(0);
            m_gsSound.PlaySample(SAMPLE_OPTION);
            m_bWasKeyReleased = FALSE;
        }
        break;
    // Was the up key pressed?
    case GSK_N:
        if (m_bWasKeyReleased)
        {
            // Highlight no option.
            m_gsMenu.SetHighlight(1);
            m_gsSound.PlaySample(SAMPLE_OPTION);
            m_bWasKeyReleased = FALSE;
        }
        break;
    // Was the enter key pressed?
    case GSK_ENTER:
        if (m_bWasKeyReleased)
        {
            // Remeber the option selected.
            m_nOptionSelected = m_gsMenu.GetHighlight();
            m_bWasKeyReleased = FALSE;
        }
        break;
    // Was the escape key pressed?
    case GSK_ESCAPE:
        if (m_bWasKeyReleased)
        {
            // The no option was selected.
            m_nOptionSelected = 1;
            m_bWasKeyReleased = FALSE;
        }
        break;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Get Mouse Input //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    RECT rcOption;

    // Has the mouse cursor moved from its previous position?
    if ((m_nOldMouseX != m_gsMouse.GetX()) || (m_nOldMouseY != m_gsMouse.GetY()))
    {
        // Check each option to see if the mouse coordinates are within the option area.
        for (int nLoop = 0; nLoop < m_gsMenu.GetNumOptions(); nLoop++)
        {
            // Determine the screen coordinates of the next option.
            m_gsMenu.GetOptionRect(nLoop, &rcOption);
            // Are the mouse coordinates within the option area?
            if (m_gsCollide.IsCoordInRect(m_gsMouse.GetX(), m_gsMouse.GetY(), rcOption))
            {
                // Is the option not highlighted already?
                if (nLoop != m_gsMenu.GetHighlight())
                {
                    // Highlight the selected option.
                    m_gsMenu.SetHighlight(nLoop);
                    // Play the appropriate sound effect.
                    m_gsSound.PlaySample(SAMPLE_OPTION);
                }
            }
        }
        // Assign new mouse coordinates.
        m_nOldMouseX = m_gsMouse.GetX();
        m_nOldMouseY = m_gsMouse.GetY();
    }

    // Is the left mouse button not pressed?
    if (!m_gsMouse.IsLeftPressed())
    {
        m_bWasMouseReleased = TRUE;
    }

    // Was the left mouse button pressed?
    if ((m_gsMouse.IsLeftPressed()) && (m_bWasMouseReleased))
    {
        // Determine the screen coordinates of the highlighted option.
        m_gsMenu.GetHighlightRect(&rcOption);
        // Check if mouse coordinates are within the area of the highligted option.
        if (m_gsCollide.IsCoordInRect(m_gsMouse.GetX(), m_gsMouse.GetY(), rcOption))
        {
            // Remeber which option is highlighted.
            m_nOptionSelected = m_gsMenu.GetHighlight();
            m_bWasMouseReleased = FALSE;
            // Play the appropriate sound effect.
            m_gsSound.PlaySample(SAMPLE_SELECT);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Process Menu Commands ////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Act depending on option selected.
    switch (m_nOptionSelected)
    {
    // Determine wether to increase levels or not.
    case 0:
        // Stop playing the game.
        m_nGameProgress = PLAY_OUTRO;
        // Play appropriate sound.
        m_gsSound.PlaySample(SAMPLE_SELECT);
        break;
    // Determine wether to preview drops or not.
    case 1:
        // Continue playing the game.
        m_nGameProgress = PLAY_GAME;
        // Play appropriate sound.
        m_gsSound.PlaySample(SAMPLE_SELECT);
        break;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Rendering //////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Clear the screen to the specified color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset display.
    glLoadIdentity();

    // Setup the display for rendering in 2D.
    m_gsDisplay.BeginRender2D(this->GetWindow());

    // Render the background image.
    this->RenderBackground(0.5f);

    // Render the frame rate.
    this->RenderFrameRate(0.5f);

    // Center the menu horizontally and vertically on the screen.
    m_gsMenu.SetDestX((INTERNAL_RES_X - m_gsMenu.GetWidth())  / 2);
    m_gsMenu.SetDestY((INTERNAL_RES_Y - m_gsMenu.GetHeight()) / 2);

    // Render the menu to the back surface using the specified colors.
    m_gsMenu.Render(m_gsMenuTextColor, m_gsMenuHighColor);

    // Render the mouse cursor.
    this->RenderCursor();

    // Reset display after rendering in 2D.
    m_gsDisplay.EndRender2D();

    // Swap buffers to display results.
    SwapBuffers(this->GetDevice());

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Cleanup ////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Are we leaving this section?
    if (m_nGameProgress != PLAY_EXIT)
    {
        // Remember where we've come from.
        m_nPrevProgress = PLAY_EXIT;
        // Reset method variables.
        m_bIsInitialized  = FALSE;
    }

    return TRUE;
}

//==============================================================================================
// GS_Demo::PlayOutro():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successfull and FALSE if not.
//==============================================================================================

BOOL GS_Demo::PlayOutro()
{
    int nHeroDeathQuotes[] =
    {
        SAMPLE_HERO_DEATH_QUOTE_1 + m_heroSampleIndex,
        SAMPLE_HERO_DEATH_QUOTE_2 + m_heroSampleIndex,
        SAMPLE_HERO_DEATH_QUOTE_3 + m_heroSampleIndex
    };
    static int nStartScrollX, nStartScrollY;

    // Has the display not been initialized?
    if (!this->m_gsDisplay.IsReady())
    {
        return FALSE;
    }

    // Has the game been paused?
    if (this->IsPaused())
    {
        // Unpause the game.
        this->Pause(FALSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Initialization /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Has the method not been initialized?
    if (m_bIsInitialized == FALSE)
    {
        // Initialize method variables.
        m_fInterval = 0.0f;
        m_nCounter  = 0;
        m_fAlpha    = 1.0f;
        m_fScale    = 0.1f;

        // Set starting coordinates for shaking effect
        nStartScrollX = m_gsBackSprite.GetScrollX();
        nStartScrollY = m_gsBackSprite.GetScrollY();

        // Initialization completed.
        m_bIsInitialized = TRUE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Logic //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Determine the interval required to perform an action 60 times every second at the
    // current frame rate, and add it to the previous intervals.
    m_fInterval += this->GetActionInterval(60);

    // Have all the intervals added up enough to perform an action?
    while (m_fInterval >= 1.0f)
    {
        // Increase counter.
        m_nCounter++;

        // Act depending on how many counts have elapsed.
        if( (m_nCounter < DEATH_SHAKE_LENGTH) && (m_nPrevProgress != PLAY_EXIT) )
        {
            // Set the shake amount (modify it to decrease over time)
            float fShakeMod = DEATH_SHAKE_STRENGTH - (DEATH_SHAKE_STRENGTH * (float)m_nCounter / (float)DEATH_SHAKE_LENGTH);
            float fShakeX = nStartScrollX + rand()%DEATH_SHAKE_STRENGTH - fShakeMod;
            float fShakeY = nStartScrollY + rand()%DEATH_SHAKE_STRENGTH - fShakeMod;

            // Shake the background to simulate impact
            m_gsBackSprite.SetScrollXY(fShakeX, fShakeY);
        }
        else
        {
            // Stop shaking the background
            m_gsBackSprite.SetScrollXY(nStartScrollX, nStartScrollY);
        }

        // Act depending on how many counts have elapsed.
        if (m_nCounter < 125)
        {
            // Delay the hero's death quote a bit
            if( (m_nCounter == 15) && (m_nPrevProgress != PLAY_EXIT) )
            {
                // Play the hero death quote
                m_gsSound.PlaySample(nHeroDeathQuotes[rand()%3]);
            }

            // Are images not entirely transparent?
            if (m_fAlpha > 0.0f)
            {
                // Increase the transparency.
                m_fAlpha -= 0.01f;
                // Is the game music playing?
                if (m_gsSound.IsStreamPlaying())
                {
                    // Fade the game music volume.
                    m_gsSound.SetStreamVolume(m_gsSettings.nMusicVolume * m_fAlpha);
                }
            }
            else
            {
                // Images are transparent.
                m_fAlpha = 0.0f;
            }
        }
        else
        {
            // Is the score good enough to be a hiscore?
            if (m_lScore > m_gsHiscores[MAX_SCORES-1].lScore)
            {
                // Add the score to the hiscore table.
                m_nNextProgress = SCORES_ADD;
                m_nGameProgress = SCORES_INTRO;
            }
            else
            {
                // Go to the view hiscore section.
                m_nNextProgress = SCORES_VIEW;
                m_nGameProgress = SCORES_INTRO;
            }
        }

        // Increase game over scale
        if( m_fScale < 4.0f )
        {
            m_fScale += 0.1f;
        }

        // One action less to be taken.
        m_fInterval -= 1.0f;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Rendering //////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Clear the screen to the specified color.
    glClearColor(m_fAlpha, m_fAlpha, m_fAlpha, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset display.
    glLoadIdentity();

    // Setup the display for rendering in 2D.
    m_gsDisplay.BeginRender2D(this->GetWindow());

    // Render the background image.
    this->RenderBackground(m_fAlpha);

    // Render the frame rate.
    this->RenderFrameRate(m_fAlpha);

    // Display a message centered in the screen.
    m_gsFont.SetText("GAME OVER");
    m_gsFont.SetScaleXY(m_fScale, m_fScale);
    m_gsFont.SetDestX((INTERNAL_RES_X -  m_gsFont.GetTextWidth()) / 2);
    m_gsFont.SetDestY((INTERNAL_RES_Y - m_gsFont.GetTextHeight()) / 2);
    m_gsFont.SetModulateColor(-1.0f, -1.0f, -1.0f, 1.0f - m_fAlpha);
    m_gsFont.Render();

    // Reset font size
    m_gsFont.SetScaleXY(1.0f, 1.0f);

    // Reset display after rendering in 2D.
    m_gsDisplay.EndRender2D();

    // Swap buffers to display results.
    SwapBuffers(this->GetDevice());

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Cleanup ////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Are we leaving this section?
    if (m_nGameProgress != PLAY_OUTRO)
    {
        // Remember where we've come from.
        m_nPrevProgress = PLAY_OUTRO;
        // Reset all variables used in the game.
        m_gsBackSprite.SetScaleXY(1.0f, 1.0f);
        m_gsBackSprite.SetRotateX(0.0f);
        m_gsBackSprite.SetRotateY(0.0f);
        m_gsBackSprite.SetRotateZ(0.0f);
        // Set the sound effects volume to game settings volume.
        m_gsSound.SetSampleMaster(m_gsSettings.nEffectsVolume);
        // Set the music volume to the game settings volume.
        m_gsSound.SetStreamVolume(m_gsSettings.nMusicVolume);
        // Is the game music playing?
        if (m_gsSound.IsStreamPlaying())
        {
            // Restore the game music volume.
            m_gsSound.SetStreamVolume( m_gsSettings.nMusicVolume);
            // Stop game music from playing.
            m_gsSound.StopStream();
        }
        // Reset method variables.
        m_bIsInitialized = FALSE;
    }

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////
// HI-SCORE METHODS ////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


//==============================================================================================
// GS_Demo::ScoresIntro():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successfull and FALSE if not.
//==============================================================================================

BOOL GS_Demo::ScoresIntro()
{

    // Has the display not been initialized?
    if (!this->m_gsDisplay.IsReady())
    {
        return FALSE;
    }

    // Has the game been paused?
    if (this->IsPaused())
    {
        // Unpause the game.
        this->Pause(FALSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Initialization /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Has the method not been initialized?
    if (m_bIsInitialized == FALSE)
    {
        // Are we not going to the add hiscore section?
        if (m_nNextProgress != SCORES_ADD)
        {
            // Go to view hiscores section by default.
            m_nNextProgress = SCORES_VIEW;
        }
        // Initialization completed.
        m_bIsInitialized = TRUE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Logic //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Do we want to view hiscores?
    if (m_nNextProgress == SCORES_VIEW)
    {
        // ...
    }
    // Do we want to add hiscores?
    else if (m_nNextProgress == SCORES_ADD)
    {
        // ...
    }

    // ...

    // Progress to the next section.
    m_nGameProgress = m_nNextProgress;

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Rendering //////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Clear the screen to the specified color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset display.
    glLoadIdentity();

    // Setup the display for rendering in 2D.
    m_gsDisplay.BeginRender2D(this->GetWindow());

    // ...

    // Reset display after rendering in 2D.
    m_gsDisplay.EndRender2D();

    // Swap buffers to display results.
    SwapBuffers(this->GetDevice());

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Cleanup ////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Are we leaving this section?
    if (m_nGameProgress != SCORES_INTRO)
    {
        // Remember where we've come from.
        m_nPrevProgress = SCORES_INTRO;
        // Reset method variables.
        m_bIsInitialized = FALSE;
    }

    return TRUE;
}

//==============================================================================================
// GS_Demo::ScoresView():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successfull and FALSE if not.
//==============================================================================================

BOOL GS_Demo::ScoresView()
{

    // Has the display not been initialized?
    if (!this->m_gsDisplay.IsReady())
    {
        return FALSE;
    }

    // Has the game been paused?
    if (this->IsPaused())
    {
        // Unpause the game.
        this->Pause(FALSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Initialization /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    char szTempString[128] = { 0 };

    // Has the method not been initialized?
    if (m_bIsInitialized == FALSE)
    {
        // Initialize variables.
        m_fInterval = 0.0f;
        // Clear the keyboard buffer.
        m_gsKeyboard.ClearBuffer();
        // Where we want to go to next.
        m_nNextProgress = SCORES_OUTRO;
        // Initialization completed.
        m_bIsInitialized = TRUE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Logic //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Determine the interval required to perform an action 60 times every second at the
    // current frame rate, and add it to the previous intervals.
    m_fInterval += this->GetActionInterval(60);

    // Have counter reached specified count?
    if (m_fInterval >= 900.0f)
    {
        // Progress to the next section.
        m_nGameProgress = m_nNextProgress;
        // Reset time counter.
        m_fInterval = 0.0f;
    }

    // Exit method if user presses any key.
    if (m_gsKeyboard.GetBufferedKey() > 0)
    {
        // Progress to the next section.
        m_nGameProgress = m_nNextProgress;
    }

    // Is the left mouse button not pressed?
    if (!m_gsMouse.IsLeftPressed())
    {
        m_bWasMouseReleased = TRUE;
    }

    // Was the left mouse button pressed?
    if ((m_gsMouse.IsLeftPressed()) && (m_bWasMouseReleased))
    {
        // Exit method if user presses left mouse button
        m_nGameProgress = m_nNextProgress;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Rendering //////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Clear the screen to the specified color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset display.
    glLoadIdentity();

    // Setup the display for rendering in 2D.
    m_gsDisplay.BeginRender2D(this->GetWindow());

    // Render the background image.
    this->RenderBackground(0.5f);

    // Calculate the place to start drawing the scores
    int scoreStartY =
        ((INTERNAL_RES_Y - m_gsFont.GetTextHeight() * MAX_SCORES) / 2) +
        ((MAX_SCORES - 2) * m_gsFont.GetTextHeight());

    // Setup and display the hiscores heading.
    sprintf(szTempString, " TOP %02d HEROES ", MAX_SCORES);
    m_gsFont.SetText(szTempString);
    m_gsFont.SetModulateColor(0.5f, 0.9f, 0.8f, 1.0f);
    m_gsFont.SetScaleXY(2.0f, 2.0f);
    m_gsFont.SetDestX((INTERNAL_RES_X - m_gsFont.GetTextWidth())  / 2);
    m_gsFont.SetDestY(INTERNAL_RES_Y - ((INTERNAL_RES_Y - scoreStartY) / 2) - (m_gsFont.GetTextHeight() / 2));
    m_gsFont.Render();

    // Reset the font size
    m_gsFont.SetScaleXY(1.0f, 1.0f);

    // For every score in the hiscores array.
    for (int nLoop = 0; nLoop < MAX_SCORES; nLoop++)
    {
        // Are we at the highlighted score?
        if (nLoop == m_nScoreIndex)
        {
            // Set the highlight color
            m_gsFont.SetModulateColor(1.0f, 0.5f, 0.5f, 1.0f);
        }
        else
        {
            // Set the default color
            m_gsFont.SetModulateColor(1.0f, 1.0f, 1.0f, 1.0f);
        }
        // Setup a temporary string with all the relevant hiscore data and assign it.
        sprintf(szTempString, " %02d %s %06ld ",nLoop+1, m_gsHiscores[nLoop].szName,
                m_gsHiscores[nLoop].lScore);
        m_gsFont.SetText(szTempString);
        // Set the position of the next hiscore.
        m_gsFont.SetDestX((INTERNAL_RES_X -  m_gsFont.GetTextWidth())  / 2);
        m_gsFont.SetDestY(scoreStartY);
        // Draw the next score one line down
        scoreStartY -= m_gsFont.GetTextHeight();
        // Render the text.
        m_gsFont.Render();
    }

    // Display a message centered at the bottom of the screen.
    m_gsFont.SetText("PRESS ANY KEY");
    m_gsFont.SetDestX((INTERNAL_RES_X - m_gsFont.GetTextWidth()) / 2);
    m_gsFont.SetDestY(m_gsFont.GetTextHeight() * 2);
    m_gsFont.Render();

    // Reset display after rendering in 2D.
    m_gsDisplay.EndRender2D();

    // Swap buffers to display results.
    SwapBuffers(this->GetDevice());

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Cleanup ////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Are we leaving this section?
    if (m_nGameProgress != SCORES_VIEW)
    {
        // Remember where we've come from.
        m_nPrevProgress = SCORES_VIEW;
        // Reset method variables.
        m_nScoreIndex    = 0;
        m_bIsInitialized = FALSE;
    }

    return TRUE;
}

//==============================================================================================
// GS_Demo::ScoresAdd():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successfull and FALSE if not.
//==============================================================================================

BOOL GS_Demo::ScoresAdd()
{

    // Has the display not been initialized?
    if (!this->m_gsDisplay.IsReady())
    {
        return FALSE;
    }

    // Has the game been paused?
    if (this->IsPaused())
    {
        // Unpause the game.
        this->Pause(FALSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Initialization /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    static int nCharCount  = 0;

    static char szTempString[11] = { 0 };

    // Has the method not been initialized?
    if (m_bIsInitialized == FALSE)
    {
        // Initialize variables.
        m_nScoreIndex = 0;
        nCharCount    = 0;
        // Loop through all the hiscores starting from the lowest.
        for (int nLoop = MAX_SCORES-1; nLoop >= 0; nLoop--)
        {
            // Is the current score is greater than the hiscore?
            if (m_lScore > m_gsHiscores[nLoop].lScore)
            {
                // Save place where score should be inserted.
                m_nScoreIndex = nLoop;
            }
        }
        // Setup a temporary string for the player name.
        lstrcpy(szTempString, "..........");
        // Clear the keyboard buffer.
        m_gsKeyboard.ClearBuffer();
        // Play the hero hiscore quote
        m_gsSound.PlaySample(SAMPLE_HERO_HISCORE_QUOTE + m_heroSampleIndex);
        // Where we want to go to next.
        m_nNextProgress = SCORES_OUTRO;
        // Initialization completed.
        m_bIsInitialized = TRUE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Get Keyboard Input ///////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Check to see wether a key was pressed.
    int nKey = m_gsKeyboard.GetBufferedKey();

    // Act depending on key pressed.
    switch (nKey)
    {
    // Was the up key pressed?
    case GSK_UP:
        // Go to the next character.
        szTempString[nCharCount]++;
        // Allow only certain characters.
        if (szTempString[nCharCount] == GSK_DELETE+1)
        {
            szTempString[nCharCount] = GSK_0;
        }
        else if (szTempString[nCharCount] == GSK_9+1)
        {
            szTempString[nCharCount] = GSK_A;
        }
        else if (szTempString[nCharCount] == GSK_Z+1)
        {
            szTempString[nCharCount] = GSK_DELETE;
        }
        // Play the appropriate sound effect.
        m_gsSound.PlaySample(SAMPLE_OPTION);
        break;
    // Was the up key pressed?
    case GSK_DOWN:
        // Go to the previous character.
        szTempString[nCharCount]--;
        // Allow only certain characters.
        if (szTempString[nCharCount] == GSK_DELETE-1)
        {
            szTempString[nCharCount] = GSK_Z;
        }
        else if (szTempString[nCharCount] == GSK_A-1)
        {
            szTempString[nCharCount] = GSK_9;
        }
        else if (szTempString[nCharCount] == GSK_0-1)
        {
            szTempString[nCharCount] = GSK_DELETE;
        }
        // Play the appropriate sound effect.
        m_gsSound.PlaySample(SAMPLE_OPTION);
        break;
    // Was the left key pressed?
    case GSK_LEFT:
        // Move one character left.
        nCharCount--;
        // Wrap around edges.
        if (nCharCount < 0)
        {
            nCharCount = 9;
        }
        // Play the appropriate sound effect.
        m_gsSound.PlaySample(SAMPLE_OPTION);
        break;
    // Was the right key pressed?
    case GSK_RIGHT:
        // Move one character right.
        nCharCount++;
        // Wrap around edges.
        if (nCharCount > 9)
        {
            nCharCount = 0;
        }
        // Play the appropriate sound effect.
        m_gsSound.PlaySample(SAMPLE_OPTION);
        break;
    // Was the backspace key pressed?
    case GSK_BACK:
        // Are we not at the first letter?
        if (nCharCount > 0)
        {
            // Shift all letters one back starting at active letter.
            for (int nLoop = nCharCount-1; nLoop < 9; nLoop++)
            {
                szTempString[nLoop] = szTempString[nLoop+1];
            }
            // Delete the last letter.
            szTempString[9] = GSK_DELETE;
            // Move one character left.
            nCharCount--;
            // Play the appropriate sound effect.
            m_gsSound.PlaySample(SAMPLE_OPTION);
        }
        break;
    // Was the delete key pressed?
    case GSK_DELETE:
        // Shift all letters one back starting at active letter.
        for (int nLoop = nCharCount; nLoop < 9; nLoop++)
        {
            szTempString[nLoop] = szTempString[nLoop+1];
        }
        // Delete the last letter.
        szTempString[9] = GSK_DELETE;
        // Play the appropriate sound effect.
        m_gsSound.PlaySample(SAMPLE_OPTION);
        break;
    // Was the enter key pressed?
    case GSK_ENTER:
        // Loop through all the scores lower than the current score.
        for (int nLoop = MAX_SCORES-1; nLoop > m_nScoreIndex; nLoop--)
        {
            // Shift scores to make place for the new score.
            m_gsHiscores[nLoop] = m_gsHiscores[nLoop-1];
        }
        // Save the player name and statistics.
        if( strcmp(szTempString, "..........") == 0 )
        {
            if( m_hero.GetSelectedHero() == SELECT_HERO_COURAGE)
            {
                lstrcpy(szTempString, "COURAGE...");
            }
            else
            {
                lstrcpy(szTempString, "JOHNNY....");
            }
        }
        lstrcpy(m_gsHiscores[m_nScoreIndex].szName, szTempString);
        m_gsHiscores[m_nScoreIndex].lScore  = m_lScore;
        // Play the appropriate sound effect.
        m_gsSound.PlaySample(SAMPLE_SELECT);
        // Progress to the next section.
        m_nGameProgress = m_nNextProgress;
        break;
    // Was the escape key pressed?
    case GSK_ESCAPE:
        // Play the appropriate sound effect.
        m_gsSound.PlaySample(SAMPLE_SELECT);
        // Progress to the next section.
        m_nGameProgress = m_nNextProgress;
        m_nScoreIndex = 0;
        break;
    default:
        // Was the key pressed a recognized key (0-9, A-Z, ' ' and '.')?
        if (((nKey >= GSK_A) && (nKey <= GSK_Z)) || ((nKey >= GSK_0) && (nKey <= GSK_9)))
        {
            // Assign key and move one letter right.
            szTempString[nCharCount] = (char)nKey;
            nCharCount++;
            // Play the appropriate sound effect.
            m_gsSound.PlaySample(SAMPLE_OPTION);
        }
        else if ((nKey == GSK_GREATER) || (nKey == GSK_SPACE) || (nKey == GSK_DECIMAL))
        {
            // Assign key and move one letter right.
            szTempString[nCharCount] = GSK_DELETE;
            nCharCount++;
            // Play the appropriate sound effect.
            m_gsSound.PlaySample(SAMPLE_OPTION);
        }
        // Have we reached the end of the string?
        if (nCharCount > 9)
        {
            // Don't go beyond end of string.
            nCharCount = 9;
        }
        break;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Logic //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // ...

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Rendering //////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Clear the screen to the specified color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset display.
    glLoadIdentity();

    // Setup the display for rendering in 2D.
    m_gsDisplay.BeginRender2D(this->GetWindow());

    // Display a message centered on the screen.
    m_gsFont.SetText("ENTER YOUR NAME:");
    m_gsFont.SetDestX((INTERNAL_RES_X - m_gsFont.GetTextWidth()) / 2);
    m_gsFont.SetDestY(((INTERNAL_RES_Y - m_gsFont.GetTextHeight()) / 2) +
                      m_gsFont.GetTextHeight());
    m_gsFont.Render();

    // Display all the letters of the temporary string.
    for (int nLoop = 0; nLoop < 10; nLoop++)
    {
        // Set the screen coordinates for each letter.
        m_gsFont.SetDestX(((INTERNAL_RES_X - (m_gsFont.GetLetterWidth() * 10)) / 2) +
                          (nLoop * m_gsFont.GetLetterWidth()));
        m_gsFont.SetDestY(((INTERNAL_RES_Y - m_gsFont.GetTextHeight()) / 2) -
                          m_gsFont.GetTextHeight());
        // Are we at the currently active letter?
        if (nLoop == nCharCount)
        {
            // Set the modulate color for the active letter.
            m_gsFont.SetModulateColor(1.0f, 0.5f, 0.5f, 1.0f);
        }
        else
        {
            // Set the modulate color for all other letters.
            m_gsFont.SetModulateColor(1.0f, 1.0f, 1.0f, 1.0f);
        }
        // Render the selected letter.
        m_gsFont.RenderChar(szTempString[nLoop]);
    }

    // Reset the modulate color of the game font.
    m_gsFont.SetModulateColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Reset display after rendering in 2D.
    m_gsDisplay.EndRender2D();

    // Swap buffers to display results.
    SwapBuffers(this->GetDevice());

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Cleanup ////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Are we leaving this section?
    if (m_nGameProgress != SCORES_ADD)
    {
        // Remember where we've come from.
        m_nPrevProgress = SCORES_ADD;
        // Save the new hiscores.
        this->SaveHiscores();
        // Reset method variables.
        m_bIsInitialized = FALSE;
    }

    return TRUE;
}

//==============================================================================================
// GS_Demo::ScoresOutro():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successfull and FALSE if not.
//==============================================================================================

BOOL GS_Demo::ScoresOutro()
{

    // Has the display not been initialized?
    if (!this->m_gsDisplay.IsReady())
    {
        return FALSE;
    }

    // Has the game been paused?
    if (this->IsPaused())
    {
        // Unpause the game.
        this->Pause(FALSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Initialization /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Has the method not been initialized?
    if (m_bIsInitialized == FALSE)
    {
        // ...
        // Initialization completed.
        m_bIsInitialized = TRUE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Logic //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Did we view hiscores?
    if (m_nPrevProgress == SCORES_VIEW)
    {
        // Progress to the next section.
        m_nGameProgress = TITLE_INTRO;
    }
    // Did we add hiscores?
    else if (m_nPrevProgress == SCORES_ADD)
    {
        // Progress to the next section.
        m_nNextProgress = SCORES_VIEW;
        m_nGameProgress = SCORES_INTRO;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Rendering //////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Clear the screen to the specified color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset display.
    glLoadIdentity();

    // Setup the display for rendering in 2D.
    m_gsDisplay.BeginRender2D(this->GetWindow());

    // ...

    // Reset display after rendering in 2D.
    m_gsDisplay.EndRender2D();

    // Swap buffers to display results.
    SwapBuffers(this->GetDevice());

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Do Method Cleanup ////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Are we leaving this section?
    if (m_nGameProgress != SCORES_OUTRO)
    {
        // Remember where we've come from.
        m_nPrevProgress = SCORES_OUTRO;
        // Reset method variables.
        m_bIsInitialized = FALSE;
    }

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////
// LOAD / SAVE METHODS /////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


//==============================================================================================
// GS_Demo::LoadSettings():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successful, FALSE if failed.
//==============================================================================================

BOOL GS_Demo::LoadSettings()
{

    char szTempString[_MAX_PATH] = { 0 };
    GS_IniFile gsIniFile;

    // Determine the full pathname of the INI file.
    GetCurrentDirectory(_MAX_PATH, szTempString);
    lstrcat(szTempString, "\\");
    lstrcat(szTempString, SETTINGS_FILE);

    // Open the INI file.
    if (!gsIniFile.Open(szTempString))
    {
        // Exit function
        return FALSE;
    }

    // Read all the display settings.
    m_gsSettings.nDisplayWidth   = gsIniFile.ReadInt("Display", "DisplayWidth",   -1);
    m_gsSettings.nDisplayHeight  = gsIniFile.ReadInt("Display", "DisplayHeight",  -1);
    m_gsSettings.nColorDepth     = gsIniFile.ReadInt("Display", "ColorDepth",     -1);
    m_gsSettings.bWindowedMode   = gsIniFile.ReadInt("Display", "WindowMode",     -1);
    m_gsSettings.bEnableVSync    = gsIniFile.ReadInt("Display", "EnableVSync",    -1);
    m_gsSettings.bEnableAliasing = gsIniFile.ReadInt("Display", "EnableAliasing", -1);

    // Did reading display settings fail?
    if (-1 == m_gsSettings.nDisplayWidth)
    {
        return FALSE;
    }
    else if (-1 == m_gsSettings.nDisplayHeight)
    {
        return FALSE;
    }
    else if (-1 == m_gsSettings.nColorDepth)
    {
        return FALSE;
    }
    else if (-1 == m_gsSettings.bWindowedMode)
    {
        return FALSE;
    }
    else if (-1 == m_gsSettings.bEnableVSync)
    {
        return FALSE;
    }
    else if (-1 == m_gsSettings.bEnableAliasing)
    {
        return FALSE;
    }

    // Read all the sound settings.
    m_gsSettings.nMusicVolume   = gsIniFile.ReadInt("Sound", "MusicVolume",   -1);
    m_gsSettings.nEffectsVolume = gsIniFile.ReadInt("Sound", "EffectsVolume", -1);

    // Did reading sound settings fail?
    if (-1 == m_gsSettings.nMusicVolume)
    {
        return FALSE;
    }
    else if (-1 == m_gsSettings.nEffectsVolume)
    {
        return FALSE;
    }

    // Close the INI file.
    gsIniFile.Close();

    return TRUE;
}

//==============================================================================================
// GS_Demo::SaveSettings():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successful, FALSE if failed.
//==============================================================================================

BOOL GS_Demo::SaveSettings()
{

    char szTempString[_MAX_PATH] = { 0 };
    GS_IniFile gsIniFile;

    // Determine the full pathname of the INI file.
    GetCurrentDirectory(_MAX_PATH, szTempString);
    lstrcat(szTempString, "\\");
    lstrcat(szTempString, SETTINGS_FILE);

    // Open the INI file.
    if (!gsIniFile.Open(szTempString))
    {
        // Exit function
        return FALSE;
    }

    // Write display settings.
    gsIniFile.WriteInt("Display", "DisplayWidth",   m_gsSettings.nDisplayWidth);
    gsIniFile.WriteInt("Display", "DisplayHeight",  m_gsSettings.nDisplayHeight);
    gsIniFile.WriteInt("Display", "ColorDepth",     m_gsSettings.nColorDepth);
    gsIniFile.WriteInt("Display", "WindowMode",     m_gsSettings.bWindowedMode);
    gsIniFile.WriteInt("Display", "EnableVSync",    m_gsSettings.bEnableVSync);
    gsIniFile.WriteInt("Display", "EnableAliasing", m_gsSettings.bEnableAliasing);

    // Write sound settings.
    gsIniFile.WriteInt("Sound", "MusicVolume",   m_gsSettings.nMusicVolume);
    gsIniFile.WriteInt("Sound", "EffectsVolume", m_gsSettings.nEffectsVolume);

    // Close the INI file.
    gsIniFile.Close();

    return TRUE;
}

//==============================================================================================
// GS_Demo::LoadHiscores():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successful, FALSE if failed.
//==============================================================================================

BOOL GS_Demo::LoadHiscores()
{

    char szTempString[_MAX_PATH] = { 0 };
    GS_IniFile gsIniFile;

    // Determine the full pathname of the INI file.
    GetCurrentDirectory(_MAX_PATH, szTempString);
    lstrcat(szTempString, "\\");
    lstrcat(szTempString, HISCORES_FILE);

    // Open the INI file.
    if (!gsIniFile.Open(szTempString))
    {
        // Exit function
        return FALSE;
    }

    // For every score in the hiscores array.
    for (int nLoop = 0; nLoop < MAX_SCORES; nLoop++)
    {
        // Determine the score heading.
        sprintf(szTempString, "Score%d", nLoop);
        // Load the player name from the hiscores file.
        lstrcpy(m_gsHiscores[nLoop].szName,
                gsIniFile.ReadString(szTempString, "Name", ".........."));
        // Load the player score from the hiscores file.
        m_gsHiscores[nLoop].lScore = (long) gsIniFile.ReadFloat(szTempString, "Score", 0);
    }

    // Close the INI file.
    gsIniFile.Close();

    return TRUE;
}

//==============================================================================================
// GS_Demo::SaveHiscores():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: TRUE if successful, FALSE if failed.
//==============================================================================================

BOOL GS_Demo::SaveHiscores()
{

    char szTempString[_MAX_PATH] = { 0 };
    GS_IniFile gsIniFile;

    // Determine the full pathname of the INI file.
    GetCurrentDirectory(_MAX_PATH, szTempString);
    lstrcat(szTempString, "\\");
    lstrcat(szTempString, HISCORES_FILE);

    // Open the INI file.
    if (!gsIniFile.Open(szTempString))
    {
        // Exit function
        return FALSE;
    }

    // For every score in the hiscores array.
    for (int nLoop = 0; nLoop < MAX_SCORES; nLoop++)
    {
        // Save the score heading.
        sprintf(szTempString, "Score%d", nLoop);
        // Save the player name and score to the hiscores file.
        if( strlen(m_gsHiscores[nLoop].szName) > 0 )
            gsIniFile.WriteString(szTempString,"Name", m_gsHiscores[nLoop].szName);
        gsIniFile.WriteFloat(szTempString, "Score", (float) m_gsHiscores[nLoop].lScore);
    }

    // Close the INI file.
    gsIniFile.Close();

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//  Helper Methods  ////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


//==============================================================================================
// GS_Demo::RenderBackground():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: Nothing.
//==============================================================================================

void GS_Demo::RenderBackground(float fAlpha)
{

    // Set the alpha value to determine the transparency of the background.
    m_gsBackSprite.SetModulateColor(-1.0f, -1.0f, -1.0f, fAlpha);

    // Postion the background centered in the screen.
    m_gsBackSprite.SetDestXY((INTERNAL_RES_X - m_gsBackSprite.GetScaledWidth())  / 2,
                             (INTERNAL_RES_Y - m_gsBackSprite.GetScaledHeight()) / 2);

    // Display the background image.
    m_gsBackSprite.Render();

    // Is background semi-transparent?
    if (fAlpha != 1.0f)
    {
        // Reset the alpha value (transparency) of the background.
        m_gsBackSprite.SetModulateColor(-1.0f, -1.0f, -1.0f, 1.0f);
    }
}

//==============================================================================================
// GS_Demo::RenderCursor():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: Nothing.
//==============================================================================================

void GS_Demo::RenderCursor(float fAlpha)
{

    // Set the alpha value to determine the transparency of the cursor.
    m_gsCursorSprite.SetModulateColor(-1.0f, -1.0f, -1.0f, fAlpha);

    // Set the cursor position at the current mouse coordinates.
    m_gsCursorSprite.SetDestY(m_gsMouse.GetY() - (int)m_gsCursorSprite.GetScaledHeight() + 1);
    m_gsCursorSprite.SetDestX(m_gsMouse.GetX());

    // Display the cursor image.
    m_gsCursorSprite.Render();

    // Is cursor semi-transparent?
    if (fAlpha != 1.0f)
    {
        // Reset the alpha value (transparency) of the cursor.
        m_gsCursorSprite.SetModulateColor(-1.0f, -1.0f, -1.0f, 1.0f);
    }
}

//==============================================================================================
// GS_Demo::RenderFrameRate():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: Nothing.
//==============================================================================================

void GS_Demo::RenderFrameRate(float fAlpha)
{

    // Set the transparency of the font (0.0f is totally transparent).
    m_gsFont.SetModulateColor(-1.0f, -1.0f, -1.0f, fAlpha);

    // Display the frame rate at the top right of the screen
    m_gsFont.SetText("%0.2f", this->GetCurrentFrameRate());
    m_gsFont.SetScaleXY(0.5f, 0.5f);
    m_gsFont.SetDestX(INTERNAL_RES_X - m_gsFont.GetTextWidth() - m_gsFont.GetLetterWidth());
    m_gsFont.SetDestY(INTERNAL_RES_Y - (m_gsFont.GetTextHeight() * 2));
    m_gsFont.Render();

    // Reset the default font values
    m_gsFont.SetScaleXY(1.0f, 1.0f);

    // Is font semi-transparent?
    if (fAlpha != 1.0f)
    {
        // Reset the alpha value (transparency) of the font.
        m_gsFont.SetModulateColor(-1.0f, -1.0f, -1.0f, 1.0f);
    }
}

//==============================================================================================
// GS_Demo::GetActionInterval():
// ---------------------------------------------------------------------------------------------
// Purpose: Set the values used for upscaling or downscaling when rendering
// ---------------------------------------------------------------------------------------------
// Returns: Nothing
//==============================================================================================

void GS_Demo::SetRenderScaling(int nWidth, int nHeight, bool bKeepAspect)
{
    // Is no scaling required?
    if( (nWidth == INTERNAL_RES_X) && (nHeight == INTERNAL_RES_Y) )
    {
        // Reset the default scaling values
        GS_OGLDisplay::SetScaleFactorX(1.0f);
        GS_OGLDisplay::SetScaleFactorY(1.0f);

        GS_OGLDisplay::SetRenderModX(0.0f);
        GS_OGLDisplay::SetRenderModY(0.0f);

        return;
    }

    // Determine the aspect ratio of the internal resolution
    float aspect = (float)INTERNAL_RES_X / (float)INTERNAL_RES_Y;

    // Set up default values for upscaling or downscaling
    float fAspectWidth  = (float)nWidth;
    float fAspectHeight = (float)nHeight;
    float fRenderModX = 0;
    float fRenderModY = 0;

    // Should we keep the aspect ratio?
    if( bKeepAspect )
    {
        // Is the horizontal dimension greater than the vertical?
        if( INTERNAL_RES_X >= INTERNAL_RES_Y )
        {
            // Try to scale the rendering to the given width
            fAspectWidth  = (float)nWidth;
            fAspectHeight = (float)fAspectWidth / aspect;
            fRenderModX = 0;
            fRenderModY = (nHeight - fAspectHeight) / 2.0f;

            // Have we scaled beyond the given height
            if( fAspectHeight > (float)nHeight )
            {
                // Scale the rendering to the given height
                fAspectHeight = (float)nHeight;
                fAspectWidth  = fAspectHeight * aspect;
                fRenderModX = (nWidth - fAspectWidth) / 2.0f;
                fRenderModY = 0;
            }
        }
        else
        {
            // Try to scale the rendering to the given height
            fAspectHeight = (float)nHeight;
            fAspectWidth  = fAspectHeight * aspect;
            fRenderModX = (nWidth - fAspectWidth) / 2.0f;
            fRenderModY = 0;

            // Have we scaled beyond the given width
            if( fAspectWidth > (float)nWidth )
            {
                // Scale the rendering to the given width
                fAspectWidth  = (float)nWidth;
                fAspectHeight = (float)fAspectWidth / aspect;
                fRenderModX = 0;
                fRenderModY = (nHeight - fAspectHeight) / 2.0f;
            }
        }
    }

    // Set the amount with which the rendering coordinates should be modified
    GS_OGLDisplay::SetRenderModX(int(fRenderModX));
    GS_OGLDisplay::SetRenderModY(int(fRenderModY));

    // Set the scale factor to effect upscaling or downscaling depending on the resolution
    GS_OGLDisplay::SetScaleFactorX((float)fAspectWidth  / (float)INTERNAL_RES_X);
    GS_OGLDisplay::SetScaleFactorY((float)fAspectHeight / (float)INTERNAL_RES_Y);
}

//==============================================================================================
// GS_Demo::GetActionInterval():
// ---------------------------------------------------------------------------------------------
// Purpose: ...
// ---------------------------------------------------------------------------------------------
// Returns: The interval into which any action or actions must be divided in order to be
//          completed in a second at the current frame rate of the application.
//==============================================================================================

float GS_Demo::GetActionInterval(float fActionsPerSecond)
{

    // Convert frame time to seconds and multiply it by the number of actions to determine
    // what fraction of the action needs to be completed in order to complete the action in
    // one second (at the current frame rate).
    return ((this->GetFrameTime() / 1000) * fActionsPerSecond);
}

// GS_Error::Report("GS_DEMO.CPP", 441, "Break Point!");
