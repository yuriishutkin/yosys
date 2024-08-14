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

#include <errno.h>
#include "kernel/yosys.h"
#include "frontends/ast/ast.h"

#include "rtlil_exporter.h"

USING_YOSYS_NAMESPACE
using namespace RTLIL_EXPORTER;

YOSYS_NAMESPACE_BEGIN

namespace RTLIL_EXPORTER
{

export_attribute_cb export_attribute_cb_f;
export_parameter_cb export_parameter_cb_f;
export_parameter_val_cb export_parameter_val_cb_f;
export_wire_cb export_wire_cb_f;
export_memory_cb export_memory_cb_f;
export_cell_mod_cb export_cell_cb_f;
export_cell_mod_cb export_cell_mod_cb_f;
export_cell_expr_cb export_cell_expr_cb_f;
export_cell_expr_ff_cb export_cell_expr_ff_cb_f;
export_cell_connection_cb export_cell_connection_cb_f;
export_cell_connection_numbered_cb export_cell_connection_numbered_cb_f;
export_proc_cb export_proc_cb_f;
export_conn_cb export_conn_cb_f;
export_module_cb export_module_start_cb_f;
export_module_cb export_module_end_cb_f;

std::string idString_str(const RTLIL::IdString &idString) { return idString.str(); }
int const_as_int(const RTLIL::Const &c) { return c.as_int(); }
int sigspec_as_int(const RTLIL::SigSpec &sigspec) { return sigspec.as_int(); }
int sigchunk_as_int(const RTLIL::SigChunk &sigchunk) { return RTLIL::Const(sigchunk.data).as_int(); }
bool sigspec_is_wire(const RTLIL::SigSpec &sigspec) { return sigspec.is_wire(); }
const RTLIL::Wire* sigspec_as_wire(const RTLIL::SigSpec &sigspec) { return sigspec.as_wire(); }
const std::vector<RTLIL::SigChunk> &sigspec_chunks(const RTLIL::SigSpec &sigspec) { return sigspec.chunks(); }
const RTLIL::SigSpec &cell_port(const RTLIL::Cell *cell, const std::string &port) { return cell->getPort("\\" + port); }
const RTLIL::SigSpec &cell_out_port(const RTLIL::Cell *cell) { return cell->getPort(ID::Y); }

std::string full_module_name(const std::string &mname, const std::vector<param_value> &param_values, bool top) {
	std::string res = std::string("\\") + mname;

	if (top) {
		// top module is renamed back to non-parametrized string after hierarchy pass
		return res;
	}
	
	if (!param_values.empty()) {
		std::vector<std::pair<RTLIL::IdString, RTLIL::Const>> parameters;
		for (auto &para : param_values) {
			SigSpec sig_value;
			if (!RTLIL::SigSpec::parse(sig_value, NULL, para.value))
				log_warning("Can't decode value '%s'!\n", para.value.c_str());
			parameters.push_back(std::make_pair(RTLIL::escape_id(para.name), sig_value.as_const()));
		}
		res = AST::derived_module_name(res, parameters);
	}
	return res;
}

std::string item_source(const RTLIL::AttrObject *item) {
	for (auto attr = item->attributes.begin(); attr != item->attributes.end(); ++attr) {
		if (attr->first.str() == "\\src") {
			return attr->second.decode_string();
		}
	}
	return "";
}

export_status module_parameters(const std::string &mname, std::vector<std::string> &params)
{
	if (!yosys_design) {
		return EXPORT_NOT_FOUND;
	}
	
	for (auto module : yosys_design->modules()) {
		if (module->name.str().substr(1) == mname) {
			for (const auto &p : module->avail_parameters) {
				params.push_back(p.str().substr(1));
			}
			return EXPORT_OK;
		}
	}

	return EXPORT_NOT_FOUND;
}

export_status export_module_int(const std::string &full_name, const std::vector<param_value> &param_values = {});

export_status export_module_int(RTLIL::Module *module, const std::vector<param_value> &param_values = {})
{
	log("Exporting module %s\n", module->name.str().substr(1).c_str());
	for (auto param : param_values) {
		log("param %s = %s\n", param.name.c_str(), param.value.c_str());
	}
	export_status rc = export_module_start_cb_f(module);

	if (rc != EXPORT_OK) return rc;

	for (auto it = module->attributes.begin(); it != module->attributes.end(); ++it) {
		export_attribute_cb_f(module, *it);
	}

	if (!module->avail_parameters.empty()) {
		for (const auto &p : module->avail_parameters) {
			std::string value;
			bool value_set = false;

			if (!param_values.empty()) {
				std::string pname = p.str().substr(1);
				// first check if param is set from outside
				for (size_t i = 0; i < param_values.size(); i++) {
					if (pname == param_values[i].name) {
						value = param_values[i].value;
						value_set = true;
						break;
					}
				}
			}

			// next, take default param value
			if (!value_set) {
				const auto &it = module->parameter_default_values.find(p);
				if (it != module->parameter_default_values.end()) {
					if (int intval = it->second.as_int()) {
						char buf[128];
						itoa(intval, buf, 10);
						value = buf;
					} else {
						value = it->second.as_string();
					}
					value_set = true;
				}
			}

			if (value_set) {
				export_parameter_val_cb_f(module, p, value);
			} else {
				export_parameter_cb_f(module, p);
			}
		}
	}

	for (auto it : module->wires()) {
		export_wire_cb_f(module, it);
	}

	for (auto it : module->memories) {
		export_memory_cb_f(module, it.second);
	}

	for (auto cell : module->cells()) {
		if (cell->is_mem_cell()) {
			log_warning("Export not implemented: mem_cell\n");
			continue;
		}
		
		for (auto attr = cell->attributes.begin(); attr != cell->attributes.end(); ++attr) {
			export_attribute_cb_f(cell, *attr);
		}

		if (cell->type.begins_with("$") && !cell->type.begins_with("$paramod")) {
			if (cell->type.begins_with("$__") || cell->type.begins_with("$fmcombine") || cell->type.begins_with("$verific$") ||
			    cell->type.begins_with("$array:") || cell->type.begins_with("$extern:")) {
				log_warning("Export not implemented: %s\n", cell->type.c_str());
				continue;
			}

			if (RTLIL::builtin_ff_cell_types().count(cell->type)) {
				FfData ff(nullptr, cell);
				export_cell_expr_ff_cb_f(module, cell, ff);
			} else {
				export_cell_expr_cb_f(module, cell);
			}
		} else {
			export_module_int(cell->type.str());

			export_cell_mod_cb_f(module, cell);

			for (auto it = cell->connections().begin(); it != cell->connections().end(); ++it) {
				const RTLIL::IdString &portName = it->first;
				const RTLIL::SigSpec &connection = it->second;
				if (portName[0] == '$') {
					// numbered port
					int n = atoi(portName.substr(1).c_str());
					export_cell_connection_numbered_cb_f(module, cell, n, connection);
				} else {
					export_cell_connection_cb_f(module, cell, portName, connection);
				}
			}
		}
	}

	for (auto it : module->processes) {
		export_proc_cb_f(module, it.second);
	}

	for (auto it = module->connections().begin(); it != module->connections().end(); ++it) {
		export_conn_cb_f(module, it->first, it->second);
	}

	return export_module_end_cb_f(module);
}

export_status export_module_int(const std::string &full_name, const std::vector<param_value> &param_values)
{
	if (!yosys_design) {
		return EXPORT_NOT_FOUND;
	}

	for (auto module : yosys_design->modules()) {
		if (module->name.str() == full_name) {
			return export_module_int(module, param_values);
		}
	}
	// module not found
	return EXPORT_NOT_FOUND;
}

export_status export_module(const std::string& name, const std::vector<param_value> &param_values)
{
	return export_module_int(std::string("\\") + name, param_values);
}

} // namespace RTLIL_EXPORTER

YOSYS_NAMESPACE_END
