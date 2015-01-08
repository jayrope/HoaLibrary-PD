/*
// Copyright (c) 2012-2014 Eliott Paris, Julien Colafrancesco & Pierre Guillot, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#include "../hoa.pd.h"
#include "../ThirdParty/HoaLibrary/Sources/Hoa.hpp"
using namespace hoa;

typedef struct _hoa_projector
{
    t_edspobj                   f_obj;
    Projector<Hoa2d, t_sample>* f_projector;
    t_sample*                   f_ins;
    t_sample*                   f_outs;
} t_hoa_projector;

t_eclass *hoa_projector_class;

extern void *hoa_projector_new(t_symbol *s, long argc, t_atom *argv)
{
    int	order = 1;
    int numberOfPlanewaves = 4;
    t_hoa_projector *x = NULL;
    x = (t_hoa_projector *)eobj_new(hoa_projector_class);
	if (x)
	{
		if(atom_gettype(argv) == A_LONG)
			order = pd_clip_min(atom_getlong(argv), 1);
        if(atom_gettype(argv+1) == A_LONG)
			numberOfPlanewaves = pd_clip_min(atom_getlong(argv+1), order * 2 + 1);
		
		x->f_projector = new Projector<Hoa2d, t_sample>(order, numberOfPlanewaves);
		
        eobj_dspsetup(x, x->f_projector->getNumberOfHarmonics(), x->f_projector->getNumberOfPlanewaves());
        
		x->f_ins = new t_sample[x->f_projector->getNumberOfHarmonics() * HOA_MAXBLKSIZE];
        x->f_outs = new t_sample[x->f_projector->getNumberOfPlanewaves() * HOA_MAXBLKSIZE];
	}
    
	return (x);
}

extern t_hoa_err hoa_getinfos(t_hoa_projector* x, t_hoa_boxinfos* boxinfos)
{
	boxinfos->object_type = HOA_OBJECT_2D;
	boxinfos->autoconnect_inputs = x->f_projector->getNumberOfHarmonics();
	boxinfos->autoconnect_outputs = x->f_projector->getNumberOfPlanewaves();
	boxinfos->autoconnect_inputs_type = HOA_CONNECT_TYPE_AMBISONICS;
	boxinfos->autoconnect_outputs_type = HOA_CONNECT_TYPE_PLANEWAVES;
	return HOA_ERR_NONE;
}

extern void hoa_projector_perform(t_hoa_projector *x, t_object *dsp64, t_sample **ins, long numins, t_sample **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    for(long i = 0; i < numins; i++)
    {
        cblas_scopy(sampleframes, ins[i], 1, x->f_ins+i, numins);
    }
	for(long i = 0; i < sampleframes; i++)
    {
        x->f_projector->process(x->f_ins + numins * i, x->f_outs + numouts * i);
    }
    for(long i = 0; i < numouts; i++)
    {
        cblas_scopy(sampleframes, x->f_outs+i, numouts, outs[i], 1);
    }
}

extern void hoa_projector_dsp(t_hoa_projector *x, t_object *dsp, short *count, double samplerate, long maxvectorsize, long flags)
{
    object_method(dsp, gensym("dsp_add"), x, (method)hoa_projector_perform, 0, NULL);
}

extern void hoa_projector_free(t_hoa_projector *x)
{
	eobj_dspfree(x);
	delete x->f_projector;
    delete [] x->f_ins;
	delete [] x->f_outs;
}


extern "C" void setup_hoa0x2e2d0x2eprojector_tilde(void)
{
    t_eclass* c;
    
    c = eclass_new("hoa.2d.projector~", (method)hoa_projector_new, (method)hoa_projector_free, (short)sizeof(t_hoa_projector), 0L, A_GIMME, 0);
    class_addcreator((t_newmethod)hoa_projector_new, gensym("hoa.projector~"), A_GIMME, 0);
    
    eclass_dspinit(c);
    hoa_initclass(c, (method)hoa_getinfos);
    eclass_addmethod(c, (method)hoa_projector_dsp,     "dsp",      A_CANT, 0);
    
    eclass_register(CLASS_OBJ, c);
    hoa_projector_class = c;
}
