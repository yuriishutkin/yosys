/*
 *  yosys -- Yosys Open SYnthesis Suite
 *
 *  Copyright (C) 2012  Claire Xenia Wolf <claire@yosyshq.com>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *  ---
 *
 *  A very simple and straightforward backend for the RTLIL text
 *  representation.
 *
 */

#ifndef RTLIL_EXPORTER_H
#define RTLIL_EXPORTER_H

#include "kernel/yosys.h"
#include "kernel/ff.h"
#include <stdio.h>

YOSYS_NAMESPACE_BEGIN

namespace RTLIL_EXPORTER {
	
	typedef int (*export_attribute_cb)(const RTLIL::AttrObject *obj, const std::pair<RTLIL::IdString, RTLIL::Const> &attr);
	typedef int (*export_parameter_cb)(RTLIL::Module *module, const RTLIL::IdString& param);
	typedef int (*export_parameter_val_cb)(RTLIL::Module *module, const RTLIL::IdString &param, const RTLIL::Const &val);
	typedef int (*export_wire_cb)(RTLIL::Module *module, const RTLIL::Wire *wire);
	typedef int (*export_memory_cb)(RTLIL::Module *module, const RTLIL::Memory *mem);
	typedef int (*export_cell_cb)(RTLIL::Module *module, const RTLIL::Cell *cell);
	typedef int (*export_cell_expr_ff_cb)(RTLIL::Module *module, const RTLIL::Cell *cell, const FfData &ffdata);
	typedef int (*export_proc_cb)(RTLIL::Module *module, const RTLIL::Process *proc);
	typedef int (*export_conn_cb)(RTLIL::Module *module, const RTLIL::SigSpec &left, const RTLIL::SigSpec &right);
	typedef int (*export_module_cb)(RTLIL::Module *module, RTLIL::Design *design);
	typedef int (*export_design_cb)(RTLIL::Design *design);

	extern YOSYS_DLL_SPEC export_attribute_cb export_attribute_cb_f;
	extern YOSYS_DLL_SPEC export_parameter_cb export_parameter_cb_f;
	extern YOSYS_DLL_SPEC export_parameter_val_cb export_parameter_val_cb_f;
	extern YOSYS_DLL_SPEC export_wire_cb export_wire_cb_f;
	extern YOSYS_DLL_SPEC export_memory_cb export_memory_cb_f;
	extern YOSYS_DLL_SPEC export_cell_cb export_cell_cb_f;
	extern YOSYS_DLL_SPEC export_cell_cb export_cell_expr_cb_f;
	extern YOSYS_DLL_SPEC export_cell_expr_ff_cb export_cell_expr_ff_cb_f;
	extern YOSYS_DLL_SPEC export_proc_cb export_proc_cb_f;
	extern YOSYS_DLL_SPEC export_conn_cb export_conn_cb_f;
	extern YOSYS_DLL_SPEC export_module_cb export_module_start_cb_f;
	extern YOSYS_DLL_SPEC export_module_cb export_module_end_cb_f;
	extern YOSYS_DLL_SPEC export_design_cb export_design_cb_f;

	YOSYS_DLL_SPEC std::string idString_str(const RTLIL::IdString& idString);
	YOSYS_DLL_SPEC const std::vector<RTLIL::SigChunk> &sigspec_chunks(const RTLIL::SigSpec &sigspec);
	YOSYS_DLL_SPEC int const_as_int(const RTLIL::Const &c);
	YOSYS_DLL_SPEC int sigspec_as_int(const RTLIL::SigSpec &sigspec);
	YOSYS_DLL_SPEC int sigchunk_as_int(const RTLIL::SigChunk &sigchunk);
	YOSYS_DLL_SPEC bool sigspec_is_wire(const RTLIL::SigSpec &sigspec);
	YOSYS_DLL_SPEC const RTLIL::Wire* sigspec_as_wire(const RTLIL::SigSpec &sigspec);
	inline std::string sigspec_name(const RTLIL::SigSpec &sigspec) { return idString_str(sigspec_as_wire(sigspec)->name); }
	YOSYS_DLL_SPEC const RTLIL::SigSpec &cell_port(const RTLIL::Cell *cell, const std::string &port);
	YOSYS_DLL_SPEC const RTLIL::SigSpec &cell_out_port(const RTLIL::Cell *cell);
	YOSYS_DLL_SPEC std::string item_source(const RTLIL::AttrObject *item);

	YOSYS_DLL_SPEC void export_module(RTLIL::Module *module, RTLIL::Design *design);
	YOSYS_DLL_SPEC void export_design(RTLIL::Design *design);
	}

YOSYS_NAMESPACE_END

#endif
