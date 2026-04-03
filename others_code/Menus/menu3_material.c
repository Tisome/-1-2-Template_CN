// main_menu.c
#include "menu.h"
#include "data.h"
#include "dac.h"
#include "bsp_gp22_handler.h"
#include "elog.h"
// 定义结构体存储不同材料的配置


int material_handler(void* parameter)
{
    int pip_index = *(int*)parameter;
    g_parameters.pipe_type = pip_index;
    dac_set_fir_vol(g_materialConfigs[GetMaterialConfigIndex()][g_parameters.pipe_type].dac_fire);
    gp22_app_modify_reg_attr(GP22_CONF_REG2, g_materialConfigs[GetMaterialConfigIndex()][g_parameters.pipe_type].mask,
                                                                 g_parameters.pipe_type);	
    g_parameters.is_saved = 1;
    SaveParameters(&g_parameters);
    return 0;
}
int menu3_material_handler(void * parameter) {
     // 材质设置
    static const MenuItem fliuid_items[] = {
        {"PLASTIC", material_handler, " 塑料 "},
        {"METAL", material_handler, " 金属 "},
        {"ALLOY", material_handler, " 合金 "}
    };
    
    // 主菜单结构
    Menu Materialsetting_menu = {
        .title[0] = "Material Type",
        .title[1] = " 材质设置 ",
        .items_count = sizeof(fliuid_items)/sizeof(fliuid_items[0]),
        .items = fliuid_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start =g_parameters.pipe_type,
        .type  = 1
    };
    
    return display_menu(&Materialsetting_menu);
}