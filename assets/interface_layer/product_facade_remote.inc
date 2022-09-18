 <?php

    // Include
    require_once dirname(__DIR__).'/entities/data_transfer_layer/product_DTO.inc';
    require_once dirname(__DIR__).'/business_logic_layer/product_facade.inc';

    class ProductFacadeRemote {

        // Private Fields
        private $product_facade;
        private $product_DTO;
        private $error_message;

        // Default Constructor
        function __construct(){
            $this->product_facade = new ProductFacade();
            $this->product_DTO = new ProductDTO();
            $this->error_message = "";
        }

        // Default Destructor
        function __destruct() {
            
        }

        // METHODS
        function is_update_valid($post){
            if(empty($post['productPassword']) && empty($post['productEmail'])){
                return true;
            }
            if(!empty($post['productPassword']) && !empty($post['productEmail'])){
                return $this->is_password($post) && $this->is_email($post);
            }
            if(empty($post['productPassword']) && !empty($post['productEmail'])){
                return $this->is_email($post);
            }
            if(!empty($post['productPassword']) && empty($post['productEmail'])){
                return $this->is_password($post);
            }
        }

        function is_create_valid($post){
            if($this->is_empty($post['productName']) ||
                $this->is_empty($post['productDescription']) ||
                $this->is_empty($post['productUnitCost']) ||
                $this->is_empty($post['productQuantityOnHand']) ||
                $this->is_empty($post['productSupplier'])){
                    return false;
                }else{
                    return true;
                }
        }

        function is_empty($attribute){
            //Check if email is empty.
            if (empty($attribute)) {
                //Add error message.
                $this->error_message .= '<li>Field cannot be blank.</li>';
                return true;
            }
            return false;
        }

        function has_error(){
            //Check if error message exsits.
            if ($this->error_message != "") {
                return true;
            }
            return false;
        }

        function print_error(){
            //Print error message.
            echo "<div id='scrnOverlay' style='display:block;'>
                    <section id='errWin' class='window' style='display:block;'>
                    <ul id='ul-err' style='text-align:center;padding: 0;'>
                    ".$this->error_message."
                    </ul>
                    <br/>
                    <a href='' style='position:relative;vertical-align:super;'><button type='button' id='errBtn' class='btn'>Okay</button></a>
                    </section>
                </div>";
        }

        function create_product_record($DTO){
            return $this->product_facade->create_product_record($DTO);
        }

        function read_product_record($attribute, $search_string){
            $this->product_facade->read_product_record($attribute, $search_string);
        }

        function update_product_record($DTO){
            return $this->product_facade->update_product_record($DTO);
        }

        function delete_product_record($attribute, $search_string){
            return $this->product_facade->delete_product_record($attribute, $search_string);
        }

        // Getters
        function get_product_DTO(){
            return $this->product_DTO;
        }
        function get_product_facade(){
            return $this->product_facade;
        }

        // Setters
        function set_product_DTO($product_DTO) {
            $this->product_DTO = $product_DTO;
        }
        function set_error_message($error) {
            $this->error_message .= $error;
        }


    }

?> 