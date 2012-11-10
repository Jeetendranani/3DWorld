// 3D World - OpenGL CS184 Computer Graphics Project
// by Frank Gennari
// 5/12/02

#ifndef _TREE_H_
#define _TREE_H_

#include "3DWorld.h"
#include "memory_alloc.h"


unsigned const CYLIN_CACHE_ENTRIES  = 4;
unsigned const BRANCH_CACHE_ENTRIES = 3;


struct blastr; // forward reference

// small tree classes
enum {TREE_CLASS_NONE=0, TREE_CLASS_PINE, TREE_CLASS_DECID, TREE_CLASS_PALM, TREE_CLASS_DETAILED, NUM_TREE_CLASSES};


struct tree_leaf { // size = 28 + 48 = 76

	int shadow_bits;
	float color, lred, lgreen;
	vector3d norm;
	point pts[4];

	tree_leaf() : shadow_bits(0) {}
	void create_init_color(bool deterministic);
	colorRGB calc_leaf_color(colorRGBA const &leaf_color, colorRGBA const &base_color) const;
	float get_norm_scale(unsigned pt_ix) const;
	point get_center() const {return 0.25*(pts[0] + pts[1] + pts[2] + pts[3]);} // average of all 4 leaf points
};


inline bool comp_leaf(const tree_leaf &A, const tree_leaf &B) {
	return (A.pts[0].mag_sq() < B.pts[0].mag_sq());
}


struct draw_cylin : public cylinder_3dw { // size = 35 (36)

	unsigned char level;
	unsigned short branch_id;

	draw_cylin() : level(0), branch_id(0) {}
	unsigned get_num_div() const {return (N_CYL_SIDES >> 1) - ((level - 1) << 2);}
	bool can_merge(draw_cylin const &c) const {return (level == c.level && branch_id == c.branch_id);}
};


struct tree_cylin : public draw_cylin { // size = 55 (56)

	float length, deg_rotate;
	vector3d rotate;

	void assign_params(unsigned char lev, unsigned short bid, float r1_, float r2_, float len, float drot) {
		level = lev; branch_id = bid; r1 = r1_; r2 = r2_; length = len; deg_rotate = drot;
	}
};


struct tree_branch { // size = 12

	tree_cylin *cylin;
	float total_length;
	short num_cylins, num_branches;

	void clear_num() {num_cylins = num_branches = 0;}
};


class tree { // size = BIG

	static reusable_mem<tree_cylin >   cylin_cache [CYLIN_CACHE_ENTRIES ];
	static reusable_mem<tree_branch>   branch_cache[BRANCH_CACHE_ENTRIES];
	static reusable_mem<tree_branch *> branch_ptr_cache;

	vector<int> branch_cobjs, leaf_cobjs;

	typedef vert_norm_comp_color leaf_vert_type_t;
	typedef vert_norm_comp_tc branch_vert_type_t;

	int type, created, trseed1, trseed2, branch_vbo, branch_ivbo, leaf_vbo;
	bool no_delete, reset_leaves, leaves_changed, not_visible;
	vector<leaf_vert_type_t> leaf_data;
	point tree_center;
	float sphere_center_zoff, sphere_radius, init_deadness, deadness, damage;
	vector<draw_cylin> all_cylins;
	colorRGBA color, base_color, leaf_color, bcolor;
	tree_branch base, *branches_34[2], **branches;
	int base_num_cylins, ncib;
	int num_1_branches, num_big_branches_min, num_big_branches_max;
	int num_2_branches_min, num_2_branches_max;
	int num_34_branches[2], num_3_branches_min, num_3_branches_max;
	int tree_slimness, tree_wideness, base_break_off;
	float base_radius, base_length_min, base_length_max, base_curveness;
	float branch_curveness, branch_upwardness, branch_distribution, branch_1_distribution;
	float base_var, num_cylin_factor, base_cylin_factor;
	float branch_1_var, branch_1_rad_var, branch_1_start, branch_2_var, branch_2_rad_var, branch_2_start;
	float branch_4_max_radius, rotate_factor;
	float angle_rotate, branch_min_angle, branch_max_angle, branch_1_random_rotate;
	float max_2_angle_rotate, max_3_angle_rotate;  //max angle to rotate 3rd order branches around from the 2nd order branch

	//branch_4 specs
	float branch_4_distribution;
	int num_4_branches_per_occurance, num_4_cylins;
	float branch_4_rad_var, branch_4_var, branch_4_length;

	//leaves specs
	vector<tree_leaf> leaves;
	int num_min_leaves, num_max_leaves, leaf_min_angle, leaf_max_angle;
	float num_leaves_per_occ, damage_scale;
	unsigned num_branch_quads, num_unique_pts;

	coll_obj &get_leaf_cobj(unsigned i) const;
	void copy_all_leaf_colors();
	void update_leaf_orients();
	bool has_leaf_data() const {return !leaf_data.empty();}
	bool has_no_leaves() const {return (leaves.empty() || deadness >= 1.0 || init_deadness >= 1.0);}
	void get_abs_leaf_pts(point pts[4], unsigned ix) const {UNROLL_4X(pts[i_] = leaves[ix].pts[i_]+tree_center;)}
	void create_leaf_obj(unsigned ix) const;
	point sphere_center() const {return (tree_center + vector3d(0.0, 0.0, sphere_center_zoff));}

	bool is_over_mesh() const;
	bool is_visible_to_camera() const;
	void gen_leaf_color();
	colorRGB get_leaf_color(unsigned i) const;
	void burn_leaves();
	void blast_damage(blastr const *const blast_radius);
	void lightning_damage(point const &ltpos);
	void drop_leaves();
	void remove_leaf(unsigned i, bool update_data);
	bool damage_leaf(unsigned i, float damage_done);
	void draw_tree_branches(shader_t const &s, float size_scale);
	void draw_tree_leaves(shader_t const &s, float size_scale);
	float gen_bc_size(float branch_var);
	float gen_bc_size2(float branch_var);
	void gen_next_cylin(tree_cylin &cylin, tree_cylin &lcylin, float var, float rad_var, int level, int branch_id, bool rad_var_test);
	void gen_first_cylin(tree_cylin &cylin, tree_cylin &src_cylin, float bstart, float rad_var, float rotate_start, int level, int branch_id);
	void create_1_order_branch(int base_cylin_num, float rotate_start, int branch_num);
	void create_2nd_order_branch(int i, int j, int cylin_num, bool branch_deflected, int rotation);
	void create_3rd_order_branch(int i, int j, int cylin_num, int branch_num, bool branch_deflected, int rotation);
	void gen_b4(tree_branch &branch, int &branch_num, int i, int k);
	void create_4th_order_branches();
	void generate_4th_order_branch(tree_branch &src_branch, int j, float rotate_start, float temp_deg, int branch_num);
	void process_cylins(tree_cylin const *const cylins, unsigned num);
	void create_leaves_and_one_branch_array();
	void add_leaves_to_cylin(tree_cylin const &cylin, float tsize);
	void mark_leaf_changed(unsigned i);
	void copy_color(colorRGB const &color, unsigned i);
	void change_leaf_color(colorRGBA &base_color, unsigned i);

public:
	tree() : created(0), branch_vbo(0), branch_ivbo(0), leaf_vbo(0), no_delete(0), reset_leaves(0),
		leaves_changed(0), not_visible(0), sphere_center_zoff(0.0), num_branch_quads(0), num_unique_pts(0) {}
	void gen_tree(point const &pos, int size, int ttype, int calc_z, bool add_cobjs);
	void regen_tree(point const &pos, int recalc_shadows);
	void calc_leaf_shadows();
	void gen_tree_shadows(unsigned light_sources);
	void add_tree_collision_objects();
	void remove_collision_objects();
	void clear_vbo();
	void draw_tree(shader_t const &s, bool draw_branches, bool draw_leaves, bool shadow_only);
	void shift_tree(vector3d const &vd) {tree_center += vd;}
	int delete_tree();
	int get_type() const {return type;}
	point const &get_center() const {return tree_center;}
	bool get_no_delete() const {return no_delete;}
	void set_no_delete(bool no_delete_) {no_delete = no_delete_;}
};


struct tree_cont_t : public vector<tree> {

	void remove_cobjs();
	void draw_branches_and_leaves(shader_t const &s, bool draw_branches, bool draw_leaves, bool shadow_only);
	void check_leaf_shadow_change();
	void draw(bool shadow_only);
	unsigned delete_all();
	unsigned scroll_trees(int ext_x1, int ext_x2, int ext_y1, int ext_y2);
	void post_scroll_remove();
	void gen_deterministic(int ext_x1, int ext_x2, int ext_y1, int ext_y2);
	void shift_by(vector3d const &vd);
	void add_cobjs();
	void clear_vbos();
};


// function prototypes - tree
float get_tree_z_bottom(float z, point const &pos);
void remove_tree_cobjs();
void draw_trees(bool shadow_only=0);
void delete_trees();
void regen_trees(bool recalc_shadows, bool keep_old);
void shift_trees(vector3d const &vd);
void add_tree_cobjs();
void clear_tree_vbos();

// function prototypes - small trees
int add_small_tree(point const &pos, float height, float width, int tree_type, bool calc_z);
void add_small_tree_coll_objs();
void remove_small_tree_cobjs();
void gen_small_trees();
void draw_small_trees(bool shadow_only);
void shift_small_trees(vector3d const &vd);


#endif // _TREE_H_

