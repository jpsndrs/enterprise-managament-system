 <?php

    // Include
    require_once dirname(__DIR__).'/config/settings.inc';
    require_once dirname(__DIR__).'/entities/data_access_layer/purchase_DAO.inc';
    require_once dirname(__DIR__).'/entities/data_access_layer/purchase_details_DAO.inc';
    require_once dirname(__DIR__).'/entities/data_transfer_layer/purchase_DTO.inc';
    require_once dirname(__DIR__).'/entities/data_transfer_layer/purchase_details_DTO.inc';

    class SaleFacade {

        // Private Fields
        private $db_conn;
        private $purchase_DAO;
        private $purchase_details_DAO;
        private $purchase_details_DAO_array;

        // Default Constructor
        function __construct(){
            $this->db_conn = new ConnectDB();
            $this->purchase_DAO = new PurchaseDAO();
            $this->purchase_details_DAO = new PurchaseDetailsDAO();
            //$this->purchase_details_DAO_array = array();
        }

        // Default Destructor
        function __destruct() {
            //$this->db_conn->close_conn();
        }

        // METHOD
        function create_sale_record($purchase_DTO, $purchase_details_DTO){

            // Set DAOs
            $this->set_purchase_DAO($this->purchase_DTO2DAO($purchase_DTO));
            $this->set_purchase_details_DAO($this->purchase_details_DTO2DAO($purchase_details_DTO));

            // Get Stock Level for product
            $command = "SELECT IF(productQuantityOnHand > ".$this->purchase_details_DAO->get_quantity().", TRUE, FALSE) AS bool FROM product WHERE productID = ".$this->purchase_details_DAO->get_productID().";";
            $result = mysqli_query($this->db_conn->get_conn(), $command);
            if(!$result){
                return false;
            }
            $row = $result->fetch_assoc();

            if(!$row["bool"]){
                $result->free_result();
                //Stock too low
                echo '<script type="text/javascript">alert("Stock too low");</script>';
                return false;
            }
            
            // Turn autocommit off
            $this->db_conn->get_conn()->autocommit(FALSE);
            // Start Transaction
            $this->db_conn->get_conn()->begin_transaction();

            // Insert into purchase.
            $command = "INSERT INTO purchase (memberID) VALUES ('".$this->purchase_DAO->get_memberID()."');";
            if(!mysqli_query($this->db_conn->get_conn(), $command)){
                $this->db_conn->get_conn()->rollback();
                // Turn autocommit on
                $this->db_conn->get_conn()->autocommit(TRUE);
                return false;
            }
                        
            // Set purchaseID.
            $command = "INSERT INTO purchased_products (purchaseID, productID, quantity, subTotal) VALUES
                ( ".$this->db_conn->get_conn()->insert_id.",
                ".$this->purchase_details_DAO->get_productID()." ,
                ".$this->purchase_details_DAO->get_quantity()." ,
                (select p.productUnitCost from product AS p WHERE p.productID = ".$this->purchase_details_DAO->get_productID().")*".$this->purchase_details_DAO->get_quantity().");";
            if(!mysqli_query($this->db_conn->get_conn(), $command)){
                $this->db_conn->get_conn()->rollback();
                // Turn autocommit on
                $this->db_conn->get_conn()->autocommit(TRUE);
                return false;
            }

            // Get current stock level.
            $command = "SET @stock_level = (select productQuantityOnHand from product WHERE productID = ".$this->purchase_details_DAO->get_productID().");";
            if(!mysqli_query($this->db_conn->get_conn(), $command)){
                $this->db_conn->get_conn()->rollback();
                // Turn autocommit on
                $this->db_conn->get_conn()->autocommit(TRUE);
                return false;
            }
            // Set new stock level.
            $command = "UPDATE product AS p SET p.productQuantityOnHand = @stock_level - ".$this->purchase_details_DAO->get_quantity()." WHERE p.productID = ".$this->purchase_details_DAO->get_productID().";";
            if(!mysqli_query($this->db_conn->get_conn(), $command)){
                $this->db_conn->get_conn()->rollback();
                // Turn autocommit on
                $this->db_conn->get_conn()->autocommit(TRUE);
                return false;
            }
        
            /* If code reaches this point without errors then commit the data in the database */
            $this->db_conn->get_conn()->commit();
            // Turn autocommit on
            $this->db_conn->get_conn()->autocommit(TRUE);
            return true;
        }

        function read_sale_record($attribute, $search_string){
            $command = "SELECT * FROM purchase WHERE $attribute = '$search_string';";
            //Return the result from the query.
            $result = mysqli_query($this->db_conn->get_conn(), $command);
            $row = mysqli_fetch_assoc($result);
            if($row != NULL){
                
                $this->set_purchase_DAO($this->purchase_DAO->with_row($row));

                $command = "SELECT * FROM purchased_products WHERE $attribute = '$search_string';";
                //Return the result from the query.
                $result = mysqli_query($this->db_conn->get_conn(), $command);
                $row = mysqli_fetch_all($result);
                if($row != NULL){
                    $temp_array = array();
                    for($i = 0; $i < count($row); $i++){
                        $temp_array[$i] = new PurchaseDetailsDAO();
                        $temp_array[$i] = $this->purchase_details_DAO->with_row_all($row[$i]);
                    }
                    $this->purchase_details_DAO_array = array_values($temp_array);
                    $test =  $this->get_purchase_details_DAO_array();

                    return true;
                }else{
                    return false;
                }
            }else{
                return false;
            }
        }

        function update_sale_record($purchase_DTO, $purchase_details_DTO){

            // Set DAOs
            $this->set_purchase_DAO($this->purchase_DTO2DAO($purchase_DTO));
            $this->set_purchase_details_DAO($this->purchase_details_DTO2DAO($purchase_details_DTO));

            // Get Stock Level for product
            $command = "SELECT IF(productQuantityOnHand > ".$this->purchase_details_DAO->get_quantity().", TRUE, FALSE) AS bool FROM product WHERE productID = ".$this->purchase_details_DAO->get_productID().";";
            $result = mysqli_query($this->db_conn->get_conn(), $command);
            if(!$result){
                return false;
            }
            $row = $result->fetch_assoc();

            if(!$row["bool"]){
                $result->free_result();
                //Stock too low
                echo '<script type="text/javascript">alert("Stock too low");</script>';
                return false;
            }
            
            // Turn autocommit off
            $this->db_conn->get_conn()->autocommit(FALSE);
            // Start Transaction
            $this->db_conn->get_conn()->begin_transaction();
                        
            // Set purchaseID.
            $command = "INSERT INTO purchased_products (purchaseID, productID, quantity, subTotal) VALUES
                ( ".$purchase_DTO->get_purchaseID().",
                ".$this->purchase_details_DAO->get_productID()." ,
                ".$this->purchase_details_DAO->get_quantity()." ,
                (select p.productUnitCost from product AS p WHERE p.productID = ".$this->purchase_details_DAO->get_productID().")*".$this->purchase_details_DAO->get_quantity().");";
            if(!mysqli_query($this->db_conn->get_conn(), $command)){
                $this->db_conn->get_conn()->rollback();
                // Turn autocommit on
                $this->db_conn->get_conn()->autocommit(TRUE);
                return false;
            }

            // Get current stock level.
            $command = "SET @stock_level = (select productQuantityOnHand from product WHERE productID = ".$this->purchase_details_DAO->get_productID().");";
            if(!mysqli_query($this->db_conn->get_conn(), $command)){
                $this->db_conn->get_conn()->rollback();
                // Turn autocommit on
                $this->db_conn->get_conn()->autocommit(TRUE);
                return false;
            }
            // Set new stock level.
            $command = "UPDATE product AS p SET p.productQuantityOnHand = @stock_level - ".$this->purchase_details_DAO->get_quantity()." WHERE p.productID = ".$this->purchase_details_DAO->get_productID().";";
            if(!mysqli_query($this->db_conn->get_conn(), $command)){
                $this->db_conn->get_conn()->rollback();
                // Turn autocommit on
                $this->db_conn->get_conn()->autocommit(TRUE);
                return false;
            }
        
            /* If code reaches this point without errors then commit the data in the database */
            $this->db_conn->get_conn()->commit();
            // Turn autocommit on
            $this->db_conn->get_conn()->autocommit(TRUE);
            return true;
        }

        


        function delete_sale_record($attribute, $search_string){
            if(!$this->read_sale_record($attribute, $search_string)){
                return false;
            }else{
                $command1 = "DELETE FROM purchased_products WHERE $attribute = '$search_string';";
                $command2= "DELETE FROM purchase WHERE $attribute = '$search_string';";
                //DELETE FROM purchase WHERE $attribute = '$search_string';";
                if(mysqli_query($this->db_conn->get_conn(), $command1) && mysqli_query($this->db_conn->get_conn(), $command2)){
                    return true;
                }else{
                    return false;
                }
 
            }
  
        }

        function export_purchase_to_csv(){
            //stack overflow Franksenstein code but it works
            header('Content-Type: text/csv; charset=utf-8');  
            header('Content-Disposition: attachment; filename=purchase.csv');  
            //cleaning output buffer so whole web page isn't exported
            ob_end_clean();
            $output = fopen("php://output", "w");  
            fputcsv($output, array('Purchase ID', 'Purchase Date', 'Purchase Member ID'));  
            $command = "SELECT * FROM purchase";  
            $result = mysqli_query($this->db_conn->get_conn(), $command); 
            while($row = mysqli_fetch_assoc($result)){  
                fputcsv($output, $row);  
            }  
            fclose($output);  
            exit;
        }

        function export_purchase_details_to_csv(){
            //stack overflow Franksenstein code but it works
            header('Content-Type: text/csv; charset=utf-8');  
            header('Content-Disposition: attachment; filename=purchase_details.csv');  
            //cleaning output buffer so whole web page isn't exported
            ob_end_clean();
            $output = fopen("php://output", "w");  
            fputcsv($output, array('Purchase ID', 'Product ID', 'Quantity', 'Sub Total'));  
            $command = "SELECT * FROM purchased_products";  
            $result = mysqli_query($this->db_conn->get_conn(), $command); 
            while($row = mysqli_fetch_assoc($result)){  
                fputcsv($output, $row);  
            }  
            fclose($output);  
            exit;
        }

        private function purchase_DAO2DTO($DAO){
            $instance = new PurchaseDTO();
            $instance->set_purchaseID($DAO->get_purchaseID());
            $instance->set_date($DAO->get_date());
            $instance->set_memberID($DAO->get_memberID());
            return $instance;
        }
        private function purchase_DTO2DAO($DTO){
            $instance = new PurchaseDAO();
            $instance->set_purchaseID($DTO->get_purchaseID());
            $instance->set_date($DTO->get_date());
            $instance->set_memberID($DTO->get_memberID());
            return $instance;
        }

        private function purchase_details_DAO2DTO($DAO){
            $instance = new PurchaseDetailsDTO();
            $instance->set_productID($DAO->get_productID());
            $instance->set_purchaseID($DAO->get_purchaseID());
            $instance->set_quantity($DAO->get_quantity());
            $instance->set_sub_total($DAO->get_sub_total());
            return $instance;
        }
        private function purchase_details_DTO2DAO($DTO){
            $instance = new PurchaseDetailsDAO();
            $instance->set_productID($DTO->get_productID());
            $instance->set_purchaseID($DTO->get_purchaseID());
            $instance->set_quantity($DTO->get_quantity());
            $instance->set_sub_total($DTO->get_sub_total());
            return $instance;
        }

        // Getters
        function get_db_conn(){
            return $this->db_conn;
        }
        function get_purchase_DAO(){
            return $this->purchase_DAO;
        }
        function get_purchase_details_DAO(){
            return $this->purchase_details_DAO;
        }
        function get_purchase_DTO(){
            return $this->purchase_DAO2DTO($this->purchase_DAO);
        }
        function get_purchase_details_DTO(){
            return $this->purchase_details_DAO2DTO($this->purchase_details_DAO);
        }
        function get_purchase_details_DAO_array(){
            return $this->purchase_details_DAO_array;
        }
        function get_array_size(){
            return count($this->get_purchase_details_DAO_array());
        }
        // Setters
        function set_db_conn($db_conn) {
            $this->db_conn = $db_conn;
        }
        function set_purchase_DAO($purchase_DAO) {
            $this->purchase_DAO = $purchase_DAO;
        }
        function set_purchase_details_DAO($purchase_details_DAO) {
            $this->purchase_details_DAO = $purchase_details_DAO;
        }
    }

?> 
