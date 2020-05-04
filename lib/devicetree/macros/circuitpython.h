#ifndef DEVICETREE_CIRCUITPYTHON_H_
#define DEVICETREE_CIRCUITPYTHON_H_

#ifdef __cplusplus
extern "C" {
#endif

#define DT_MP_ROM_PTR(node_id) DT_CAT(node_id, _MP_ROM_PTR)

#define DT_INST_MP_ROM_PTR(inst) \
        DT_MP_ROM_PTR(DT_DRV_INST(inst))

#ifdef __cplusplus
}
#endif

#endif  /* DEVICETREE_CIRCUITPYTHON_H_ */
