# Aug.18 2024
- What I left
    - add death and menu
    - add items
    - add monsters
    - finetuning parameters
    - add audio
    - think about side black screen
# Aug.19 2024
- What I did
    [x] add score display
        now exceeding VBlank Time
    [x] add github repo
    [x] fix bug of consecutive fake blocks. maybe max height is too high
    [x] Figure out how to use sprites
    [x] reduce jump height to make the platform you departed not disappear below the screen when screen scrolling
    [x] change background to tilemap mode
        https://gbadev.net/tonc/regbg.html
        - think the order of backgrounds and objects
    [x] add window to background
    [x] write text on tile mode
    [x] learnt tte advanced text engine
    [x] add death and menu
        [x] fix start/init bugs
    [x] add textbox for text
    [x] fixed bug of consective fake platforms
- next plan
    - figure out how to export tilemap with transparent indice 0 being reserved
    - figure out how to set background color
# Aug.20 2024
- What I did
    [x] Add Backdrop
         Getting more familiar with tile and pal memory
         Getting familiar with ipad as external drawing tablet for pc
    - add audio
        [x] jump notes
        [x] game over song
        [x] jump noise sfx
    [x] fixed overflow bug
- Plan
    - add items
    - add monsters
    - add shotting animation
    - add bullet gameobject
- What I left
    - still consequent fake platform distance too large bug
    - fix VRAM overlap bug
# Aug.27 2024
- What I did
    [x] finished dealing with the VRAM overlap problem and rehauled the resource allocation logic
- Next
    - add propeller and rocket
    - add monsters. try to trace original onsters design
    - add shot animation
    - add bullet spawn and bullet object pool
    - fix consequent fake platform distance too large bug
# Aug.28 2024
[x] replace mod and div with and and rshift
[x] separate c code to multiple files
[x] upgraded sprite load code
- add monsters
    [temporaily use current] planning
    [x] artwork
    [x] display, memory pool and spawing
    [x] player collision
    [x] bullet collision
    [ ] death sfx
    [ ] monster movement
    - determine monster distribution in levels
- add bullets
    [x] artwork
    [x] display, memory pool and spawing
    [x] movement
    [x] shoot
    [ ] shoot sfx
- add items
    [ ] artwork
    [ ] display, memory pool and spawing
    [ ] sfx
    [ ] effects
- refactor song engine
    [ ] concurrent songs
    [ ] different instruments
- maybe new level generation paradigm?
    - sequential level templates
