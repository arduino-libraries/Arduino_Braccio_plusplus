#include <Braccio++.h>

#define MARGIN_LEFT   0
#define MARGIN_TOP    0

static const char * btnm_map[] = {"Option 1", "\n",
                                  "Option 2", "\n",
                                  "Option 3", "\n", "\0"
                                 };

void customMenu(){
  lv_obj_t * btnm1 = lv_btnmatrix_create(lv_scr_act());
  lv_btnmatrix_set_map(btnm1, btnm_map);
  lv_obj_align(btnm1, LV_ALIGN_CENTER, MARGIN_LEFT, MARGIN_TOP);
}

void setup() {
  Braccio.begin(customMenu);
}

void loop() {
  
}
