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

	struct param_value {
		std::string name;
		std::string value;
		param_value(const std::string &name, const std::string& value) : name(name), value(value) {}
	};

	enum export_status {
		EXPORT_OK = 0,
		EXPORT_NOT_FOUND = 1,
		EXPORT_NOT_IMPLEMENTED = 2,

		EXPORT_INTERNAL_ERROR = -1
	};
	
	typedef export_status (*export_attribute_cb)(const RTLIL::AttrObject *obj, const std::pair<RTLIL::IdString, RTLIL::Const> &attr);
	typedef export_status (*export_parameter_cb)(RTLIL::Module *module, const RTLIL::IdString &param);
	typedef export_status (*export_parameter_val_cb)(RTLIL::Module *module, const RTLIL::IdString &param, const std::string& val);
	typedef export_status (*export_wire_cb)(RTLIL::Module *module, const RTLIL::Wire *wire);
	typedef export_status (*export_memory_cb)(RTLIL::Module *module, const RTLIL::Memory *mem);
	typedef export_status (*export_cell_cb)(RTLIL::Module *module, const RTLIL::Cell *cell);
	typedef export_status (*export_cell_mod_cb)(RTLIL::Module *module, const RTLIL::Cell *cell);
	typedef export_status (*export_cell_expr_cb)(RTLIL::Module *module, const RTLIL::Cell *cell);
	typedef export_status (*export_cell_expr_ff_cb)(RTLIL::Module *module, const RTLIL::Cell *cell, const FfData &ffdata);
	typedef export_status (*export_cell_connection_cb)(RTLIL::Module *module, const RTLIL::Cell *cell, const RTLIL::IdString &port, const RTLIL::SigSpec &connection);
	typedef export_status (*export_cell_connection_numbered_cb)(RTLIL::Module *module, const RTLIL::Cell *cell, int port, const RTLIL::SigSpec &connection);
	typedef export_status (*export_proc_cb)(RTLIL::Module *module, const RTLIL::Process *proc);
	typedef export_status (*export_conn_cb)(RTLIL::Module *module, const RTLIL::SigSpec &left, const RTLIL::SigSpec &right);
	typedef export_status (*export_module_cb)(RTLIL::Module *module);

	extern YOSYS_DLL_SPEC export_attribute_cb export_attribute_cb_f;
	extern YOSYS_DLL_SPEC export_parameter_cb export_parameter_cb_f;
	extern YOSYS_DLL_SPEC export_parameter_val_cb export_parameter_val_cb_f;
	extern YOSYS_DLL_SPEC export_wire_cb export_wire_cb_f;
	extern YOSYS_DLL_SPEC export_memory_cb export_memory_cb_f;
	extern YOSYS_DLL_SPEC export_cell_cb export_cell_cb_f;
	extern YOSYS_DLL_SPEC export_cell_mod_cb export_cell_mod_cb_f;
	extern YOSYS_DLL_SPEC export_cell_expr_cb export_cell_expr_cb_f;
	extern YOSYS_DLL_SPEC export_cell_expr_ff_cb export_cell_expr_ff_cb_f;
	extern YOSYS_DLL_SPEC export_cell_connection_cb export_cell_connection_cb_f;
	extern YOSYS_DLL_SPEC export_cell_connection_numbered_cb export_cell_connection_numbered_cb_f;
	extern YOSYS_DLL_SPEC export_proc_cb export_proc_cb_f;
	extern YOSYS_DLL_SPEC export_conn_cb export_conn_cb_f;
	extern YOSYS_DLL_SPEC export_module_cb export_module_start_cb_f;
	extern YOSYS_DLL_SPEC export_module_cb export_module_end_cb_f;

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
	YOSYS_DLL_SPEC export_status module_parameters(const std::string &mname, std::vector<std::string> &params);

	YOSYS_DLL_SPEC export_status export_module(const std::string& name, const std::vector<param_value> &param_values = {});
	}

YOSYS_NAMESPACE_END

#endif
