#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <limits.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

const char *win_name = "Confront";
char *conf_s = NULL;
int win_w, win_h;
int win_fullscreen_ = 0;
int win_border_ = 0;

SDL_Window *win;
SDL_Surface *win_surf;
SDL_Rect win_rect;

SDL_Joystick *joy;
SDL_JoystickID joy_id;
int joy_n = 1;

#define IMG_N (3)
enum {
    IMG_BG, IMG_SEL, IMG_SEL_GO
};
SDL_Surface *cf_imgs[IMG_N];

#define ICON_MAX_N (512)
char *icon_apps[ICON_MAX_N];
SDL_Surface *icon_imgs[ICON_MAX_N];
SDL_Rect icon_rect[ICON_MAX_N];
int icon_n;
int icon_sel = 0;
int icon_sel_go = -1;

SDL_Rect sel_rect[ICON_MAX_N];

enum {
    ACT_LEFT, ACT_RIGHT, ACT_GO
};

#define SND_N (3)
enum {
    SND_SEL, SND_NO, SND_GO
};
Mix_Chunk *snds[SND_N];

int init ();
void paint ();
int react (int act);

int main (int argc, char *argv[]) {
    SDL_Event event;

    if (argc > 1) {
        for (int c = 1; c < argc; c ++) {
            if (! strcmp(argv[c], "--help")) {
                printf("Usage: %s [options]\nwhere [options] is zero or more of:\n", argv[0]);
                printf("\t--full (enable fullscreen)\n");
                printf("\t--border (enable window title/border)\n");
                printf("\t--joy=n (specify joystick number, 0 for none)\n");
                printf("\t--config=path (load alternate config file)\n");
                printf("\t--help (this message)\n");
                return 0;
            }
            if (! strcmp(argv[c], "--full"))
                win_fullscreen_ = 1;
            else if (! strcmp(argv[c], "--border"))
                win_border_ = 1;
            else if (! strncmp(argv[c], "--joy=", 6))
                joy_n = atoi(argv[c] + 6);
            else if (! strncmp(argv[c], "--config=", 9))
                conf_s = (char *)strdup(argv[c] + 9);
        }
    }

    int res = init();
    if (res) {
        printf("init failed\ntry \"%s --help\"\n", argv[0]);
        return res;
    }

    while (SDL_WaitEvent (&event) != 0) {
        if (event.type == SDL_QUIT) {
            break;
        }
        else if (event.type == SDL_WINDOWEVENT) {
            icon_sel_go = -1;
            paint();
        }
        else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_LEFT:
                    react(ACT_LEFT);
                    break;

                case SDLK_RIGHT:
                    react(ACT_RIGHT);
                    break;

                case SDLK_SPACE:
                case SDLK_RETURN:
                    react(ACT_GO);
                    break;
            }
        }
        else if (event.type == SDL_JOYHATMOTION) {
            if (event.jhat.which == joy_id) {
                if (event.jhat.value & SDL_HAT_LEFT)
                    react(ACT_LEFT);
                else if (event.jhat.value & SDL_HAT_RIGHT)
                    react(ACT_RIGHT);
            }
        }
        else if (event.type == SDL_JOYBUTTONDOWN) {
            if (event.jbutton.which == joy_id) {
                if (event.jbutton.button == 0)
                    react(ACT_GO); 
            }
        }
    }

    if (SDL_JoystickGetAttached(joy))
        SDL_JoystickClose(joy);
    Mix_Quit();
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

int init () {
    char name_s [PATH_MAX + 1];
    char *name_s2;
    FILE *f_in;
    
    if (! conf_s) {
        conf_s = (char *)malloc(strlen(CF_SHARE) + strlen("confront.conf") + 2);
        sprintf(conf_s, "%s/%s", CF_SHARE, "confront.conf");
    }

    f_in = fopen(conf_s, "r");
    if (! f_in) {
        printf("open \"%s\" failed\n", conf_s);
        return -1;
    }

    if (fscanf(f_in, "%d %d", &win_w, &win_h) == EOF) {
        printf("read window resolution failed\n");
        return -1;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK)) {
        printf("SDL_Init failed\n");
        return -1;
    }

    int win_flags = SDL_WINDOW_SHOWN;
    if (win_fullscreen_)
        win_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    if (! win_border_)
        win_flags |= SDL_WINDOW_BORDERLESS;

    win = SDL_CreateWindow(win_name,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        win_w, win_h, win_flags);
    if (! win) {
        printf("SDL_CreateWindow failed\n");
        return -1;
    }
    
    win_surf = SDL_GetWindowSurface(win);
    if (! win_surf) {
        printf("SDL_GetWindowSurface failed\n");
        return -1;
    }

    win_rect.x = win_rect.y = 0;
    win_rect.w = win_w;
    win_rect.h = win_h;

    if (joy_n) {
        joy = SDL_JoystickOpen(joy_n - 1);
        if (! joy) {
            printf("SDL_JoystickOpen %d failed\n", joy_n);
        }
        else {
            joy_id = SDL_JoystickInstanceID(joy);
            if (joy_id < 0) {
                printf("SDL_JoystickInstanceID failed\n");
                return -1;
            }
        }
    }

    Mix_OpenAudio (48000, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_Init (MIX_INIT_OGG);
    
    strncpy(name_s, CF_SHARE, PATH_MAX);
    name_s2 = name_s + strlen(CF_SHARE);
    *name_s2++ = '/';
    
    for (int c = 0; c < IMG_N; c ++) {
        if (fscanf(f_in, "%s", name_s2) == EOF) {
            printf("parse config failed\n");
            return -1;
        }

        cf_imgs[c] = IMG_Load(name_s);
        if (! cf_imgs[c]) {
            printf("IMG_Load \"%s\" failed\n", name_s);
            return -1;
        }
    }

    for (int c = 0; c < SND_N; c ++) {
        if (fscanf(f_in, "%s", name_s2) == EOF) {
            printf("parse config failed (2)\n");
            return -1;
        }

        snds[c] = Mix_LoadWAV(name_s);
        if (! snds[c]) {
            printf("Mix_LoadWAV \"%s\" failed\n", name_s);
            return -1;
        }
    }

    for (icon_n = 0; icon_n < ICON_MAX_N; icon_n++) {
        if (fscanf(f_in, "%s", name_s2) == EOF)
            break;
        icon_apps[icon_n] = (char *)strdup(name_s2); // note the s2
 
        if (fscanf(f_in, "%s", name_s2) == EOF)
            break;
        icon_imgs[icon_n] = IMG_Load(name_s);
        if (! icon_imgs[icon_n]) {
            printf("IMG_Load \"%s\" failed\n", name_s);
            return -1;
        }
    }

    fclose(f_in);
    
    int sel_w = cf_imgs[1]->w;
    int sel_x1 = (win_w / 2) - ((sel_w * icon_n) / 2);
    int sel_y1 = (win_h / 2) - (sel_w / 2);
    for (int c = 0; c < icon_n; c ++) {
        sel_rect[c].x = sel_x1 + (sel_w * c);
        sel_rect[c].y = sel_y1;
        sel_rect[c].w = sel_w;
        sel_rect[c].h = sel_w;

        int icon_off = (sel_w / 2) - (icon_imgs[c]->w / 2);
        icon_rect[c].x = sel_rect[c].x + icon_off;
        icon_rect[c].y = sel_rect[c].y + icon_off;
        icon_rect[c].w = icon_imgs[c]->w;
        icon_rect[c].h = icon_imgs[c]->w;
    }

    return 0;
}

void paint () {
    SDL_BlitSurface(cf_imgs[IMG_BG], NULL, win_surf, &win_rect);
    SDL_BlitSurface(cf_imgs[IMG_SEL], NULL, win_surf, &sel_rect[icon_sel]);
    for (int c = 0; c < icon_n; c ++)
        SDL_BlitSurface(icon_imgs[c], NULL, win_surf, &icon_rect[c]);
    if (icon_sel_go >= 0)
        SDL_BlitSurface(cf_imgs[IMG_SEL_GO], NULL, win_surf, &sel_rect[icon_sel_go]);
    SDL_UpdateWindowSurface(win);
}

int react (int act) {
    pid_t pid;

    icon_sel_go = -1;

    switch (act) {
        case ACT_LEFT:
            if (icon_sel == 0) {
                Mix_PlayChannel(0, snds[SND_NO], 0);
                return -1;
            }
            icon_sel --;
            Mix_PlayChannel(0, snds[SND_SEL], 0);
            break;

        case ACT_RIGHT:
            if (icon_sel == icon_n - 1) {
                Mix_PlayChannel(0, snds[SND_NO], 0);
                return -1;
            }
            icon_sel ++;
            Mix_PlayChannel(0, snds[SND_SEL], 0);
            break;

        case ACT_GO:
            Mix_PlayChannel(0, snds[SND_GO], 0);

            pid = fork();
            if (pid == -1) {
                printf("fork failed\n");
                return -1;
            }
            else if (pid == 0) {
                if (execlp(icon_apps[icon_sel], NULL))
                    exit(1);
            }
            else {
                icon_sel_go = icon_sel;
            }
            break;
    }

    paint();
    return 0;
}

