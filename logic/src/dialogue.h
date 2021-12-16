#pragma once

#include "common.h"
#include "core.h"

struct dialogue_script* new_dialogue_script(const char* source);
void free_dialogue_script(struct dialogue_script* script);

void play_dialogue(struct dialogue_script* script);
void update_dialogue(struct dialogue_script* script);
