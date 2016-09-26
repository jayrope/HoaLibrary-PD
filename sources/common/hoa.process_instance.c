/*
// Copyright (c) 2012-2016 Pierre Guillot, Eliott Paris & Thomas Le Meur CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#include "../hoa.pd.h"
#include <string.h>
#include <g_canvas.h>
#include <m_imp.h>

static t_symbol* hoa_sym_canvas;
static t_symbol* hoa_sym_obj;

static t_symbol* hoa_sym_hoa_thisprocess;
static t_symbol* hoa_sym_hoa_in;
static t_symbol* hoa_sym_hoa_out;
static t_symbol* hoa_sym_hoa_in_tilde;
static t_symbol* hoa_sym_hoa_out_tilde;

static t_symbol* hoa_sym_harmonics;
static t_symbol* hoa_sym_planewaves;

void hoa_process_instance_setup(void)
{
    hoa_sym_canvas          = gensym("canvas");
    hoa_sym_obj             = gensym("obj");
    
    hoa_sym_hoa_thisprocess = gensym("hoa.thisprocess~");
    hoa_sym_hoa_in          = gensym("hoa.in");
    hoa_sym_hoa_out         = gensym("hoa.out");
    hoa_sym_hoa_in_tilde    = gensym("hoa.in~");
    hoa_sym_hoa_out_tilde   = gensym("hoa.out~");
    
    hoa_sym_harmonics       = gensym("harmonics");
    hoa_sym_planewaves      = gensym("planewaves");
}

static void hoa_process_instance_get_hoas(t_hoa_process_instance* x, t_canvas* cnv)
{
    t_gobj *y;
    t_symbol const* name;
    for(y = cnv->gl_list; y; y = y->g_next)
    {
        name = y->g_pd->c_name;
        if(name == hoa_sym_canvas)
        {
            hoa_process_instance_get_hoas(x, (t_canvas *)y);
        }
        else if(name == hoa_sym_hoa_in)
        {
            t_hoa_in* inlet = (t_hoa_in *)y;
            inlet->f_next = x->f_ins;
            x->f_ins = inlet;
        }
        else if(name == hoa_sym_hoa_out)
        {
            t_hoa_out* outlet = (t_hoa_out *)y;
            outlet->f_next = x->f_outs;
            x->f_outs = outlet;
        }
        else if(name == hoa_sym_hoa_in_tilde)
        {
            t_hoa_io_tilde* inlet_sig = (t_hoa_io_tilde *)y;
            if(inlet_sig->f_extra)
            {
                inlet_sig->f_next = x->f_ins_extra_sig;
                x->f_ins_extra_sig = inlet_sig;
            }
            else
            {
                inlet_sig->f_next = x->f_ins_sig;
                x->f_ins_sig = inlet_sig;
            }
        }
        else if(name == hoa_sym_hoa_out_tilde)
        {
            t_hoa_io_tilde* outlet_sig = (t_hoa_io_tilde *)y;
            if(outlet_sig->f_extra)
            {
                outlet_sig->f_next = x->f_outs_extra_sig;
                x->f_outs_extra_sig = outlet_sig;
            }
            else
            {
                outlet_sig->f_next = x->f_outs_sig;
                x->f_outs_sig = outlet_sig;
            }
        }
    }
}

char hoa_process_instance_init(t_hoa_process_instance* x, t_canvas* parent, t_symbol* name, size_t argc, t_atom* argv)
{
    t_gobj* z;
    t_atom* av;
    x->f_canvas = NULL;
    av = (t_atom *)getbytes((argc + 3) * sizeof(t_atom));
    if(av)
    {
        SETFLOAT(av, 10);
        SETFLOAT(av+1, 10);
        SETSYMBOL(av+2, name);
        memcpy(av+3, argv, (size_t)argc * sizeof(t_atom));
        pd_typedmess((t_pd *)parent, hoa_sym_obj, (int)(argc + 3), av);
        for(z = parent->gl_list; z->g_next; z = z->g_next) {
        }
        if(z && z->g_pd->c_name == hoa_sym_canvas)
        {
            x->f_canvas = (t_canvas *)z;
            canvas_loadbang(x->f_canvas);
            
            hoa_process_instance_get_hoas(x, x->f_canvas);
            freebytes(av, (argc + 3) * sizeof(t_atom));
            return 1;
        }
        freebytes(av, (argc + 3) * sizeof(t_atom));
    }
    
    return 0;
}






void hoa_process_instance_show(t_hoa_process_instance* x)
{
    if(x->f_canvas)
    {
        canvas_vis(x->f_canvas, 1);
    }
}

void hoa_process_instance_send_bang(t_hoa_process_instance* x, size_t extra)
{
    t_hoa_in* in = x->f_ins;
    while(in != NULL)
    {
        if(in->f_extra == extra)
        {
            pd_bang((t_pd *)in);
        }
        in = in->f_next;
    }
}

void hoa_process_instance_send_float(t_hoa_process_instance* x, size_t extra, float f)
{
    t_hoa_in* in = x->f_ins;
    while(in != NULL)
    {
        if(in->f_extra == extra)
        {
            pd_float((t_pd *)in, f);
        }
        in = in->f_next;
    }
}

void hoa_process_instance_send_symbol(t_hoa_process_instance* x, size_t extra, t_symbol* s)
{
    t_hoa_in* in = x->f_ins;
    while(in != NULL)
    {
        if(in->f_extra == extra)
        {
            pd_symbol((t_pd *)in, s);
        }
        in = in->f_next;
    }
}

void hoa_process_instance_send_list(t_hoa_process_instance* x, size_t extra, t_symbol* s, int argc, t_atom* argv)
{
    t_hoa_in* in = x->f_ins;
    while(in != NULL)
    {
        if(in->f_extra == extra)
        {
            pd_list((t_pd *)in, s, argc, argv);
        }
        in = in->f_next;
    }
}

void hoa_process_instance_send_anything(t_hoa_process_instance* x, size_t extra, t_symbol* s, int argc, t_atom* argv)
{
    t_hoa_in* in = x->f_ins;
    while(in != NULL)
    {
        if(in->f_extra == extra)
        {
            pd_typedmess((t_pd *)in, s, argc, argv);
        }
        in = in->f_next;
    }
}

size_t hoa_process_instance_has_inputs(t_hoa_process_instance* x, char extra)
{
    size_t index = 0;
    t_hoa_in* in = x->f_ins;
    if(extra)
    {
        while(in != NULL)
        {
            if(in->f_extra > index)
            {
                index = in->f_extra;
            }
            in = in->f_next;
        }
    }
    else
    {
        while(in != NULL)
        {
            if(!in->f_extra)
            {
                return (size_t)-1;
            }
            in = in->f_next;
        }
    }
    return index;
}
/*


inline ulong getMaximumSignalInputExtraIndex() const
{
    ulong n = 0ul;
    for(ulong i = 0; i < f_ins_extra_sig.size(); i++)
    {
        if(ulong(f_ins_extra_sig[i]->f_extra) > n)
            n = ulong(f_ins_extra_sig[i]->f_extra);
    }
    return n;
}

inline bool hasNormalOutputs() const
{
    return !f_outs.empty();
}

inline bool hasNormalSignalOutputs() const
{
    return !f_outs_sig.empty();
}

inline ulong getMaximumOutputExtraIndex() const
{
    ulong n = 0ul;
    for(ulong i = 0; i < f_outs_extra.size(); i++)
    {
        if(ulong(f_outs_extra[i]->f_extra) > n)
            n = ulong(f_outs_extra[i]->f_extra);
    }
    return n;
}

inline ulong getMaximumSignalOutputExtraIndex() const
{
    ulong n = 0ul;
    for(ulong i = 0; i < f_outs_extra_sig.size(); i++)
    {
        if(ulong(f_outs_extra_sig[i]->f_extra) > n)
            n = ulong(f_outs_extra_sig[i]->f_extra);
    }
    return n;
}

inline void setNomalOutlet(t_outlet* outlet)
{
    for(ulong i = 0; i < f_outs.size(); i++)
    {
        f_outs[i]->f_outlet = outlet;
    }
}

inline void setExtraOutlet(t_outlet* outlet, ulong index)
{
    for(ulong i = 0; i < f_outs_extra.size(); i++)
    {
        if(ulong(f_outs_extra[i]->f_extra) == index)
        {
            f_outs_extra[i]->f_outlet = outlet;
        }
    }
}


bool prepareDsp(t_sample* in, vector<t_sample*>& ixtra, t_sample* out, vector<t_sample*>& oxtra)
{
    if(hasNormalSignalInputs())
    {
        if(!in)
        {
            bug("process don't have input signal.");
            return true;
        }
        for(size_t i = 0; i < f_ins_sig.size(); i++)
        {
            f_ins_sig[i]->f_signal = in;
        }
    }
    for(size_t i = 0; i < f_ins_extra_sig.size(); i++)
    {
        if(ulong(f_ins_extra_sig[i]->f_extra) > ixtra.size() || !ixtra[size_t(f_ins_extra_sig[i]->f_extra-1)])
        {
            bug("process don't have input signal extra %i.", f_ins_extra_sig[i]->f_extra);
            return true;
        }
        f_ins_extra_sig[i]->f_signal = ixtra[size_t(f_ins_extra_sig[i]->f_extra-1)];
    }
    if(hasNormalSignalOutputs())
    {
        if(!out)
        {
            bug("process don't have output signal.");
            return true;
        }
        for(size_t i = 0; i < f_outs_sig.size(); i++)
        {
            f_outs_sig[i]->f_signal = out;
        }
    }
    for(size_t i = 0; i < f_outs_extra_sig.size(); i++)
    {
        if(ulong(f_outs_extra_sig[i]->f_extra) > oxtra.size() || !oxtra[size_t(f_outs_extra_sig[i]->f_extra-1)])
        {
            bug("process don't have output signal extra %i.", f_outs_extra_sig[i]->f_extra);
            return true;
        }
        f_outs_extra_sig[i]->f_signal = oxtra[size_t(f_outs_extra_sig[i]->f_extra-1)];
    }
    return false;
}
 */







