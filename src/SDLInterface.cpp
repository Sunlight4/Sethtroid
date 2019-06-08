/*
 * SDLInterface.cpp
 *
 *  Created on: May 31, 2019
 *      Author: triforce
 */

#include "SDLInterface.h"
#include <iostream>
#include <fstream>
#include "Tile.h"
const int SCREEN_WIDTH = 720;
const int SCREEN_HEIGHT = 512;
unsigned int getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;
        break;

    case 2:
        return *(Uint16 *)p;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;
        break;

    case 4:
        return *(Uint32 *)p;
        break;

    default:
        return 0;
    }
}
SDLInterface::SDLInterface() {
	// TODO Auto-generated constructor stub

}

SDLInterface::~SDLInterface() {
}
int SDLInterface::InitGraphics() {
	if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) return 1;
	window = SDL_CreateWindow("Generic", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if( window == NULL ) return 2;
	renderer = SDL_CreateRenderer(window, -1, 0);
	if (renderer == NULL) return 3;
	screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, SCREEN_WIDTH, SCREEN_HEIGHT);
	if (screenTexture==NULL) return 4;
	pixels = new unsigned int[SCREEN_WIDTH * SCREEN_HEIGHT];
	for (int i=0; i<SCREEN_WIDTH; i++) {
		for (int j=0; j<SCREEN_HEIGHT; j++) {
			pixels[SCREEN_WIDTH*j+i]=(255<<24);
		}
	}
	return 0;
}
void SDLInterface::WritePixel(unsigned int data, int x, int y) {
	if (x>SCREEN_WIDTH || x<0 || y>SCREEN_HEIGHT || y<0) return;
	if ((data>>24)==0) return;
	else {
		pixels[y*SCREEN_WIDTH+x]=data;
	}
}
void SDLInterface::BlitSprite(Sprite* sprite, int x, int y) {
	if (x>SCREEN_WIDTH || y>SCREEN_HEIGHT) {
		return;
	}
	for (int i=0; i<sprite->GetWidth(); ++i) {
		for (int j=0; j<sprite->GetHeight(); ++j) {

			WritePixel(sprite->PixelAt(i, j), i+x, j+y);
		}
	}
}
bool SDLInterface::EventPoll() {
	SDL_Event e;
	SDL_PumpEvents();
	const unsigned char* state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_D]&&!state[SDL_SCANCODE_A]) {
		horizInput=1;
	}
	else if (state[SDL_SCANCODE_A]&&!state[SDL_SCANCODE_D]) {
		horizInput=-1;
	}
	else {
		horizInput=0;
	}
	isRunning=state[SDL_SCANCODE_F];
	justJumping=state[SDL_SCANCODE_SPACE]&&!isJumping;
	isJumping=state[SDL_SCANCODE_SPACE];
	while (SDL_PollEvent(&e)){
		if (e.type == SDL_QUIT){
			return false;
		}
	}
	return true;
}
int SDLInterface::CloseGraphics() {
	SDL_DestroyTexture(screenTexture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	return 0;
}
void SDLInterface::UpdateGraphics() {
	SDL_UpdateTexture(screenTexture, NULL, pixels, SCREEN_WIDTH * sizeof(unsigned int));
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
	SDL_RenderPresent(renderer);
	for (int i=0; i<SCREEN_WIDTH; i++) {
		for (int j=0; j<SCREEN_HEIGHT; j++) {
			pixels[SCREEN_WIDTH*j+i]=(255<<24);
		}
	}
}
void SDLInterface::BlitLayer(Layer* layer, int ox, int oy) {
	for (int i=0; i<SCREEN_WIDTH; ++i) {
		for (int j=0; j<SCREEN_HEIGHT; ++j) {
			WritePixel(layer->PixelAt(i+ox, j+oy), i, j);
		}
	}
}
Sprite* SDLInterface::_loadImage(char* path) {
	SDL_Surface* surface = SDL_LoadBMP(path);
	SDL_LockSurface(surface);
	Sprite* output = new Sprite(surface->w, surface->h);
	int palettesSet=0;
	for (int i=0; i<surface->w; i++) {
		for (int j=0;j<surface->h; j++) {
			unsigned int pixel = getpixel(surface, i, j);
			for (int k=0; k<palettesSet+1; k++) {
				if (k==palettesSet) {
					output->SetPalette(k, pixel);
					output->SetPixel(i, j, k);
					palettesSet++;
					break;
				}
				else {
					if (output->GetPalette(k)==pixel) {
						output->SetPixel(i, j, k);
						break;
					}
				}
			}
		}
	}

	return output;
}

Sprite* SDLInterface::_loadSprite(char* path) {
	char header[4];
	char palette[2];
	char data[1];
	std::ifstream file (path, std::ios::in | std::ios::binary);
	file.read (header, 4);
	Sprite* output = new Sprite(header[1], header[2]);
	for (int i = 0; i<header[3]; i++) {
		file.read (palette, 2);
		output->SetPalette(i, palette[0]*256+palette[1]);
	}
	for (int i = 0; i<header[1]; i++) {
		for (int j = 0; j<header[2]; j++) {
			file.read(data, 1);
			output->SetPixel(i, j, data[0]);
		}
	}
	return output;
}

void SDLInterface::_exportSprite(char* path, Sprite* sprite) {
	char header[4];
	header[0]=0;
	header[1]=sprite->GetWidth();
	header[2]=sprite->GetHeight();
	header[3]=sprite->numPalettes;
	char data[header[1]*header[2]];
	for (int i = 0; i<header[1]; i++) {
		for (int j = 0; j<header[2]; j++) {
			data[j*header[1]+i]=sprite->GetData(i, j);
		}
	}
	std::ofstream file (path, std::ios::out | std::ios::binary);
	file.write(header, 4);
	file.write(data, header[1]*header[2]);
}

AnimationResource* SDLInterface::_ImportAnimation(char* path, std::forward_list<int> widths, int interval) {
	SDL_Surface* surface = SDL_LoadBMP(path);
	if (surface==NULL) std::cout<<"!"<<std::flush;
	char**  data;
	Sprite*  output;
	SDL_LockSurface(surface);
	std::cout<<"X"<<std::flush;
	AnimationResource* anim = new AnimationResource();
	anim->interval=interval;
	int palettesSet=0;
	int sum = 0;
	for (auto it = widths.begin(); it!=widths.end(); it++) {
		output = new Sprite(*it, surface->h);
		output->palettes=anim->palettes;
		for (int i=0; i<output->GetWidth(); i++) {
			for (int j=0;j<output->GetHeight(); j++) {
				unsigned int pixel = getpixel(surface, i+sum, j);
				for (int k=0; k<palettesSet+1; k++) {
					if (k==palettesSet) {
						output->SetPalette(k, pixel);
						anim->SetPalette(k, pixel);
						output->SetPixel(i, j, k);
						palettesSet++;
						break;
					}
					else {
						if (output->GetPalette(k)==pixel) {
							output->SetPixel(i, j, k);
							break;
						}
					}
				}
			}
		}
		data = output->data;
		anim->AddFrame(data, output->GetWidth(), output->GetHeight());
		sum+=*it;
	}
	return anim;
}
void SDLInterface::_WriteAnimation(char* path, AnimationResource* animation) {
	char header[3];
	char palette[4];
	char smallHeader[2];
	std::cout<<animation->frames[0].w;
	header[0]=animation->NumFrames();
	header[1]=animation->interval;
	header[2]=animation->numPalettes;
	std::ofstream file (path, std::ios::out | std::ios::binary);
	file.write(header, 3);
	for (int i=0; i<header[2]; i++) {
		int n = animation->GetPalette(i);
		palette[0] = (n >> 24) & 0xFF;
		palette[1] = (n >> 16) & 0xFF;
		palette[2] = (n >> 8) & 0xFF;
		palette[3] = n & 0xFF;
		file.write(palette, 4);
	}
	for (int i=0; i<header[0];i++) {
		std::cout<<animation->frames[0].w;
		Frame frame = animation->frames[i];
		smallHeader[0]=frame.w;
		smallHeader[1]=frame.h;
		file.write(smallHeader, 2);
		char data[frame.w*frame.h];
		for (int i = 0; i<smallHeader[0]; i++) {
			for (int j = 0; j<smallHeader[1]; j++) {
				data[j*smallHeader[0]+i]=frame.data[i][j];
			}
		}
		file.write(data, frame.w*frame.h);
	}

}
struct HexCharStruct
{
  unsigned char c;
  HexCharStruct(unsigned char _c) : c(_c) { }
};

inline std::ostream& operator<<(std::ostream& o, const HexCharStruct& hs)
{
  return (o << std::hex << (int)hs.c);
}

inline HexCharStruct hex(unsigned char _c)
{
  return HexCharStruct(_c);
}
AnimationResource* SDLInterface::LoadAnim(char* path) {
	char header[3];
	char palette[4];
	char smallHeader[2];
	char buffer[1];
	char** data;
	std::string pth = std::string(path);
	std::map<std::string, AnimationResource*>::iterator it=anims.find(pth);
	if (it!=anims.end()) return it->second;
	AnimationResource* anim = new AnimationResource();
	std::ifstream file (path, std::ios::in | std::ios::binary);
	file.read(header, 3);
	if (header[1]!=1) std::cout<<pth<<(int)header[1]<<"\n"<<std::flush;
	//std::cout<<"\n"<<(int) header[0]<<" "<<(int) header[1]<<" "<<(int) header[2]<<"\n"<<std::flush;
	anim->interval=header[1];
	for (int i=0; i<header[2];i++) {
		file.read(palette, 4);
		anim->SetPalette(i, (palette[0]<<24)+(palette[1]<<16)+(palette[2]<<8)+palette[3]);
		//std::cout<<"\n"<<hex(palette[0])<<" "<<hex(palette[1])<<" "<<hex(palette[2])<<" "<<hex(palette[3])<<"\n"<<std::flush;
		//std::cout<<"\n"<<(palette[0]<<24)+(palette[1]<<16)+(palette[2]<<8)+palette[3]<<"\n"<<std::flush;
	}
	for (int k=0; k<header[0];k++) {
		file.read(smallHeader, 2);
		data=new char*[smallHeader[0]];
		for(int i = 0; i < smallHeader[0]; ++i) data[i] = new char[smallHeader[1]];
		for (int i = 0; i <smallHeader[0]; i++) {
			for (int j = 0; j<smallHeader[1]; j++) {
				file.read(buffer, 1);
				if (buffer[0]!=0) {
				}
				data[j][i]=buffer[0];
			}
		}
		anim->AddFrame(data, smallHeader[0], smallHeader[1]);
	}
	anims.insert(std::pair<std::string, AnimationResource*>(pth, anim));
	return anim;
}

Tileset* SDLInterface::LoadTileset(char* path) {
	char header[4];
	char palette[4];
	char buffer[1];
	std::string pth = std::string(path);
	std::map<std::string, Tileset*>::iterator it=sets.find(pth);
	if (it!=sets.end()) return it->second;
	std::ifstream file (path, std::ios::in | std::ios::binary);
	file.read(header, 4);
	Tileset* tileset = new Tileset(header[0], header[1]);
	for (int i=0; i<header[3];i++) {
		file.read(palette, 4);
		std::cout<<"\n"<<std::hex<<(palette[0]<<24)+(palette[1]<<16)+(palette[2]<<8)+palette[3]<<"\n"<<std::flush;
		tileset->SetPalette(i, (palette[0]<<24)+(palette[1]<<16)+(palette[2]<<8)+palette[3]);
			//std::cout<<"\n"<<hex(palette[0])<<" "<<hex(palette[1])<<" "<<hex(palette[2])<<" "<<hex(palette[3])<<"\n"<<std::flush;
			//std::cout<<"\n"<<(palette[0]<<24)+(palette[1]<<16)+(palette[2]<<8)+palette[3]<<"\n"<<std::flush;
	}
	char** data=new char*[header[0]];
	for(int i = 0; i < header[0]; ++i) data[i] = new char[header[1]];
	for (int i = 0; i <header[0]; i++) {
		for (int j = 0; j<header[1]; j++) {
			file.read(buffer, 1);
			data[j][i]=buffer[0];
		}
	}
	tileset->data=data;

	int k=0;

	for (int i = 0; i<header[0]/header[2]; i++) {
		for (int j = 0; j<header[1]/header[2]; j++) {
			file.read(buffer, 1);
			switch (buffer[0]) {
			case 0:
				tileset->tiles[k]=new Tile(i*header[2], j*header[2], new AABB(header[2]/2, header[2]/2), SOLID, new CollisionInfo());

				k++;
				break;
			default: break;
			}
		}
	}
	return tileset;
}
