/*++
Copyright (c) 2013 Microsoft Corporation

Module Name:

    maxsmt.h

Abstract:
   
    MaxSMT optimization context.

Author:

    Nikolaj Bjorner (nbjorner) 2013-11-7

Notes:

--*/
#ifndef _OPT_MAXSMT_H_
#define _OPT_MAXSMT_H_

#include"ast.h"
#include"params.h"
#include"solver.h"
#include"filter_model_converter.h"
#include"statistics.h"

namespace opt {

    typedef vector<rational> const weights_t;

    class context;

    class maxsmt_solver {
    public:        
        virtual ~maxsmt_solver() {}
        virtual lbool operator()() = 0;
        virtual rational get_lower() const = 0;
        virtual rational get_upper() const = 0;
        virtual bool get_assignment(unsigned index) const = 0;
        virtual void set_cancel(bool f) = 0;
        virtual void collect_statistics(statistics& st) const = 0;
        virtual void get_model(model_ref& mdl) = 0;
        virtual void updt_params(params_ref& p) = 0;

    };

    // ---------------------------------------------
    // base class with common utilities used
    // by maxsmt solvers
    // 
    class maxsmt_solver_base : public maxsmt_solver {
    protected:
        solver&          m_s;
        ast_manager&     m;
        context&         m_c;
        volatile bool    m_cancel;
        expr_ref_vector  m_soft;
        expr_ref_vector  m_assertions;
        vector<rational> m_weights;
        rational         m_lower;
        rational         m_upper;
        model_ref        m_model;
        svector<bool>    m_assignment;       // truth assignment to soft constraints
        params_ref       m_params;           // config

    public:
        maxsmt_solver_base(context& c, weights_t& ws, expr_ref_vector const& soft); 

        virtual ~maxsmt_solver_base() {}        
        virtual rational get_lower() const { return m_lower; }
        virtual rational get_upper() const { return m_upper; }
        virtual bool get_assignment(unsigned index) const { return m_assignment[index]; }
        virtual void set_cancel(bool f) { m_cancel = f; s().set_cancel(f); }
        virtual void collect_statistics(statistics& st) const { }
        virtual void get_model(model_ref& mdl) { mdl = m_model.get(); }
        void set_model() { s().get_model(m_model); }
        virtual void updt_params(params_ref& p);
        virtual void init_soft(weights_t& weights, expr_ref_vector const& soft);
        solver& s() { return m_s; }
        void init();
        expr* mk_not(expr* e);
        void set_mus(bool f);
        app* mk_fresh_bool(char const* name);
    protected:
        void enable_sls(expr_ref_vector const& soft, weights_t& ws);
    };

    /**
       Takes solver with hard constraints added.
       Returns modified soft constraints that are maximal assignments.
    */

    class maxsmt {
        ast_manager&              m;
        solver&                   m_s;
        context&                  m_c;
        scoped_ptr<maxsmt_solver> m_msolver;
        volatile bool    m_cancel;
        expr_ref_vector  m_soft_constraints;
        expr_ref_vector  m_answer;
        vector<rational> m_weights;
        rational         m_lower;
        rational         m_upper;
        model_ref        m_model;
        params_ref       m_params;
    public:
        maxsmt(context& c);
        lbool operator()(solver* s);
        void set_cancel(bool f);
        void updt_params(params_ref& p);
        void add(expr* f, rational const& w); 
        unsigned size() const { return m_soft_constraints.size(); }
        expr* operator[](unsigned idx) const { return m_soft_constraints[idx]; }
        rational weight(unsigned idx) const { return m_weights[idx]; }
        void commit_assignment();
        rational get_value() const;
        rational get_lower() const;
        rational get_upper() const;
        void update_lower(rational const& r, bool override);
        void update_upper(rational const& r, bool override);
        void get_model(model_ref& mdl);
        bool get_assignment(unsigned index) const;
        void display_answer(std::ostream& out) const;        
        void collect_statistics(statistics& st) const;
    private:
        bool is_maxsat_problem(weights_t& ws) const;        
        void verify_assignment();
    };

};

#endif
