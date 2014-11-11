#ifndef INTERACTION_H
#define INTERACTION_H

class Interaction {
public:
    //------ INTERACTION CODE STARTS HERE -----------------------------------------------------------------

    //display: Handle window redrawing events. Simply delegates to visualize().
    void display(void);
    //reshape: Handle window resizing (reshaping) events
    void reshape(int w, int h);

    //keyboard: Handle key presses
    void keyboard(unsigned char key, int x, int y);
    }

    // drag: When the user drags with the mouse, add a force that corresponds to the direction of the mouse
    //       cursor movement. Also inject some new matter into the field at the mouse location.
    void drag(int mx, int my);
}

#endif